#include "Unbounded2BoundedTransformation.hpp"

#include <rti/routing/Logger.hpp>
#include <algorithm>
#include <memory>
#include <stdexcept>

using namespace dds::core::xtypes;
using dds::sub::SampleInfo;

namespace {

void require(bool condition, const std::string &message)
{
    if (!condition) {
        throw std::invalid_argument(message);
    }
}

bool numeric_kind(TypeKind kind)
{
    switch (kind.underlying()) {
#if RTICONNEXTDDS_HAS_INT8_TYPE
    case TypeKind::INT_8_TYPE:
    case TypeKind::OCTET_TYPE:
#endif
    case TypeKind::UINT_8_TYPE:
    case TypeKind::INT_16_TYPE:
    case TypeKind::UINT_16_TYPE:
    case TypeKind::INT_32_TYPE:
    case TypeKind::UINT_32_TYPE:
    case TypeKind::INT_64_TYPE:
    case TypeKind::UINT_64_TYPE:
    case TypeKind::FLOAT_32_TYPE:
    case TypeKind::FLOAT_64_TYPE:
    case TypeKind::FLOAT_128_TYPE:
        return true;
    default:
        return false;
    }
}

struct Output {
    std::unique_ptr<DynamicData> sample;
    std::unique_ptr<SampleInfo> info;
};

template<typename Numeric>
std::vector<Numeric> read_values(DynamicData &sample, const std::string &name)
{
        return sample.get_values<Numeric>(name);
}

template<>
std::vector<rti::core::LongDouble> read_values(
                DynamicData &sample, const std::string &name)
{
        auto member = sample.loan_value(name);
        std::vector<rti::core::LongDouble> values;
        const uint32_t length = member.get().member_count();
        values.reserve(length);
        for (uint32_t index = 0; index < length; ++index) {
                values.push_back(member.get().value<rti::core::LongDouble>(index + 1));
        }
        return values;
}

template<typename Numeric>
void write_values(DynamicData &sample, const std::string &name,
                const std::vector<Numeric> &values)
{
        sample.set_values<Numeric>(name, values);
}

template<>
void write_values(DynamicData &sample, const std::string &name,
                const std::vector<rti::core::LongDouble> &values)
{
        auto member = sample.loan_value(name);
        member.get().clear_all_members();
        for (size_t index = 0; index < values.size(); ++index) {
                member.get().value<rti::core::LongDouble>(static_cast<uint32_t>(index + 1), values[index]);
        }
}

template<typename Numeric>
void append_chunks(
        std::vector<Output> &outputs,
                DynamicData &input,
        const SampleInfo &info,
        const DynamicType &output_type,
        const std::string &member_name,
        size_t bound)
{
        const auto values = read_values<Numeric>(input, member_name);
    const size_t count = values.empty()
            ? 1 : values.size() / bound + (values.size() % bound != 0);
    require(count <= outputs.max_size() - outputs.size(),
            "output chunk count exceeds vector capacity");
    size_t offset = 0;
    do {
        const size_t length = std::min(bound, values.size() - offset);
        std::vector<Numeric> chunk(values.begin() + offset,
                values.begin() + offset + length);
        Output output;
        output.sample.reset(new DynamicData(output_type));
        output.info.reset(new SampleInfo(info));
        write_values<Numeric>(*output.sample, member_name, chunk);
        offset += length;
        output.sample->value<bool>("more_data", offset < values.size());
        outputs.push_back(std::move(output));
    } while (offset < values.size());
}

}

Unbounded2BoundedTransformation::Unbounded2BoundedTransformation(
        const rti::routing::TypeInfo &input_type,
        const rti::routing::TypeInfo &output_type,
        const rti::routing::PropertySet &)
        : output_type_(output_type.dynamic_type()),
          element_kind_(TypeKind::INT_32_TYPE),
          bound_(0)
{
    try {
        const auto input = rti::core::xtypes::resolve_alias(input_type.dynamic_type());
        const auto output = rti::core::xtypes::resolve_alias(output_type_);
        require(input.kind() == TypeKind::STRUCTURE_TYPE,
                "input " + input_type.type_name() + ": expected a structure");
        require(output.kind() == TypeKind::STRUCTURE_TYPE,
                "output " + output_type.type_name() + ": expected a structure");
        const auto &input_struct = static_cast<const StructType &>(input);
        const auto &output_struct = static_cast<const StructType &>(output);
        require(!input_struct.has_parent() && input_struct.member_count() == 1,
                "input " + input.name() + ": expected exactly one member and no inheritance");
        require(!output_struct.has_parent() && output_struct.member_count() == 2,
                "output " + output.name() + ": expected exactly two members and no inheritance");
        for (const auto &member : input_struct.members()) {
            require(!member.is_key() && !member.is_optional(),
                    "input " + input.name() + ": members must be required and non-key");
        }
        for (const auto &member : output_struct.members()) {
            require(!member.is_key() && !member.is_optional(),
                    "output " + output.name() + ": members must be required and non-key");
        }
        const auto &input_member = input_struct.member(0);
        member_name_ = input_member.name().to_std_string();
        require(member_name_ != "more_data", "input sequence cannot be named more_data");
        const auto input_sequence = rti::core::xtypes::resolve_alias(input_member.type());
        const auto output_sequence = rti::core::xtypes::resolve_alias(
                output_struct.member(member_name_).type());
        const auto flag = rti::core::xtypes::resolve_alias(
                output_struct.member("more_data").type());
        require(flag.kind() == TypeKind::BOOLEAN_TYPE,
                "output more_data: expected boolean");
        require(input_sequence.kind() == TypeKind::SEQUENCE_TYPE,
                "input " + member_name_ + ": expected unbounded sequence");
        require(output_sequence.kind() == TypeKind::SEQUENCE_TYPE,
                "output " + member_name_ + ": expected bounded sequence");
        const auto &source = static_cast<const SequenceType &>(input_sequence);
        const auto &destination = static_cast<const SequenceType &>(output_sequence);
        require(source.bounds() == SequenceType::UNBOUNDED,
                "input " + member_name_ + ": expected unbounded sequence, actual bound "
                        + std::to_string(source.bounds()));
        bound_ = destination.bounds();
        require(bound_ > 0 && bound_ < SequenceType::UNBOUNDED,
                "output " + member_name_ + ": expected positive finite bound, actual "
                        + std::to_string(bound_));
        element_kind_ = rti::core::xtypes::resolve_alias(source.content_type()).kind();
        require(numeric_kind(element_kind_),
                "input " + member_name_ + ": expected numeric element, actual kind "
                        + std::to_string(element_kind_.underlying()));
        require(element_kind_ == rti::core::xtypes::resolve_alias(
                        destination.content_type()).kind(),
                "output " + member_name_ + ": numeric element kind must match input");
    } catch (const std::exception &error) {
        rti::routing::Logger::instance().error(
                std::string("Unbounded2Bounded schema: ") + error.what());
        throw;
    }
}

void Unbounded2BoundedTransformation::transform(
        std::vector<DynamicData *> &output_samples,
        std::vector<SampleInfo *> &output_infos,
        const std::vector<DynamicData *> &input_samples,
        const std::vector<SampleInfo *> &input_infos)
{
    try {
        require(output_samples.empty() && output_infos.empty(),
                "output vectors must be empty before transform");
        require(input_samples.size() == input_infos.size(),
                "input data/info counts differ");
        std::vector<Output> outputs;
        for (size_t index = 0; index < input_samples.size(); ++index) {
            require(input_infos[index] != nullptr, "null input SampleInfo");
            if (!input_infos[index]->valid()) {
                continue;
            }
            require(input_samples[index] != nullptr, "null valid input sample");
#define APPEND_CHUNKS(kind, native_type) \
            case TypeKind::kind: \
                append_chunks<native_type>(outputs, *input_samples[index], \
                        *input_infos[index], output_type_, member_name_, bound_); \
                break
            switch (element_kind_.underlying()) {
#if RTICONNEXTDDS_HAS_INT8_TYPE
                APPEND_CHUNKS(INT_8_TYPE, int8_t);
                APPEND_CHUNKS(OCTET_TYPE, uint8_t);
#endif
                APPEND_CHUNKS(UINT_8_TYPE, uint8_t);
                APPEND_CHUNKS(INT_16_TYPE, int16_t);
                APPEND_CHUNKS(UINT_16_TYPE, uint16_t);
                APPEND_CHUNKS(INT_32_TYPE, DDS_Long);
                APPEND_CHUNKS(UINT_32_TYPE, DDS_UnsignedLong);
                APPEND_CHUNKS(INT_64_TYPE, DDS_LongLong);
                APPEND_CHUNKS(UINT_64_TYPE, DDS_UnsignedLongLong);
                APPEND_CHUNKS(FLOAT_32_TYPE, float);
                APPEND_CHUNKS(FLOAT_64_TYPE, double);
                APPEND_CHUNKS(FLOAT_128_TYPE, rti::core::LongDouble);
            default:
                throw std::logic_error("unvalidated numeric kind");
            }
#undef APPEND_CHUNKS
        }
        output_samples.reserve(outputs.size());
        output_infos.reserve(outputs.size());
        for (auto &output : outputs) {
            output_samples.push_back(output.sample.release());
            output_infos.push_back(output.info.release());
        }
    } catch (const std::exception &error) {
        rti::routing::Logger::instance().error(
                std::string("Unbounded2Bounded transform: ") + error.what());
        throw;
    }
}

void Unbounded2BoundedTransformation::return_loan(
        std::vector<DynamicData *> &samples,
        std::vector<SampleInfo *> &infos)
{
    for (auto *sample : samples) {
        delete sample;
    }
    for (auto *info : infos) {
        delete info;
    }
    samples.clear();
    infos.clear();
}

Unbounded2BoundedTransformationPlugin::Unbounded2BoundedTransformationPlugin(
        const rti::routing::PropertySet &)
{
}

rti::routing::transf::Transformation *
Unbounded2BoundedTransformationPlugin::create_transformation(
        const rti::routing::TypeInfo &input_type,
        const rti::routing::TypeInfo &output_type,
        const rti::routing::PropertySet &properties)
{
    return new Unbounded2BoundedTransformation(input_type, output_type, properties);
}

void Unbounded2BoundedTransformationPlugin::delete_transformation(
        rti::routing::transf::Transformation *transformation)
{
    delete transformation;
}

RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DEF(Unbounded2BoundedTransformationPlugin)