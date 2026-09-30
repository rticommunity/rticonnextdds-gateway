#include "Unbounded2BoundedTransformation.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <set>

using namespace dds::core::xtypes;
using dds::sub::SampleInfo;

namespace {

void check(bool condition, const std::string &message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

rti::routing::TypeInfo type_info(const DynamicType &type)
{
    rti::routing::TypeInfo info("TestType");
    info.dynamic_type(&type);
    return info;
}

StructType input_type(const DynamicType &element, uint32_t bound = SequenceType::UNBOUNDED,
        const std::string &name = "data")
{
    StructType type("Input");
    type.add_member(Member(name, SequenceType(element, bound)));
    return type;
}

StructType output_type(const DynamicType &element, uint32_t bound = 3,
        const std::string &name = "data")
{
    StructType type("Output");
    type.add_member(Member("more_data", primitive_type<bool>()));
    type.add_member(Member(name, SequenceType(element, bound)));
    return type;
}

template<typename Operation>
void rejects(Operation operation, const std::string &label)
{
    bool rejected = false;
    try {
        operation();
    } catch (const std::exception &) {
        rejected = true;
    }
    check(rejected, "expected rejection: " + label);
}

void rejects_schema(const DynamicType &input, const DynamicType &output)
{
    rejects([&]() {
        Unbounded2BoundedTransformation transformation(type_info(input), type_info(output), {});
    }, "invalid schema pair");
}

template<typename Numeric>
bool equal_value(const Numeric &actual, const Numeric &expected)
{
    return actual == expected;
}

template<>
bool equal_value<float>(const float &actual, const float &expected)
{
    return (std::isnan(actual) && std::isnan(expected))
            || (actual == expected && std::signbit(actual) == std::signbit(expected));
}

template<>
bool equal_value<double>(const double &actual, const double &expected)
{
    return (std::isnan(actual) && std::isnan(expected))
            || (actual == expected && std::signbit(actual) == std::signbit(expected));
}

template<typename Numeric>
void numeric_test(const DynamicType &element, const std::vector<Numeric> &pattern)
{
    for (uint32_t bound : {1u, 3u, 1000u}) {
        auto input = input_type(element, SequenceType::UNBOUNDED, "values");
        auto output = output_type(element, bound, "values");
        Unbounded2BoundedTransformation transformation(type_info(input), type_info(output), {});
        SampleInfo info;
        info->native().valid_data = DDS_BOOLEAN_TRUE;
        info->native().source_timestamp.sec = 123;
        for (size_t length : {size_t(0), size_t(1), size_t(bound - 1), size_t(bound),
                    size_t(bound + 1), size_t(2 * bound), size_t(2 * bound + 1), size_t(1)}) {
            std::vector<Numeric> values;
            for (size_t index = 0; index < length; ++index) {
                values.push_back(pattern[index % pattern.size()]);
            }
            DynamicData sample(input);
            {
                auto member = sample.loan_value("values");
                for (size_t index = 0; index < values.size(); ++index) {
                    member.get().value<Numeric>(static_cast<uint32_t>(index + 1), values[index]);
                }
            }
            std::vector<DynamicData *> samples;
            std::vector<SampleInfo *> infos;
            transformation.transform(samples, infos, {&sample, &sample}, {&info, &info});
            const size_t chunks = length == 0 ? 1 : length / bound + (length % bound != 0);
            check(samples.size() == chunks * 2 && infos.size() == samples.size(), "chunk count");
            check(std::set<DynamicData *>(samples.begin(), samples.end()).size() == samples.size(),
                    "distinct sample ownership");
            check(std::set<SampleInfo *>(infos.begin(), infos.end()).size() == infos.size(),
                    "distinct info ownership");
            for (size_t input_index = 0; input_index < 2; ++input_index) {
                size_t offset = 0;
                for (size_t chunk_index = 0; chunk_index < chunks; ++chunk_index) {
                    const size_t index = input_index * chunks + chunk_index;
                    std::vector<Numeric> chunk;
                    {
                        auto member = samples[index]->loan_value("values");
                        for (uint32_t element_index = 0; element_index < member.get().member_count(); ++element_index) {
                            chunk.push_back(member.get().value<Numeric>(element_index + 1));
                        }
                    }
                    check(chunk.size() == std::min(size_t(bound), length - offset), "chunk length");
                    check(samples[index]->value<bool>("more_data") == (chunk_index + 1 < chunks),
                            "completion flag");
                    check(infos[index]->valid() && infos[index]->source_timestamp().sec() == 123,
                            "copied metadata");
                    for (const auto &value : chunk) {
                        check(equal_value(value, values[offset++]), "numeric value fidelity");
                    }
                }
                check(offset == length, "reconstructed length");
            }
            transformation.return_loan(samples, infos);
            check(samples.empty() && infos.empty(), "returned loan vectors");
        }
    }
}

template<typename Numeric>
void integer_test()
{
    numeric_test<Numeric>(primitive_type<Numeric>(),
            {std::numeric_limits<Numeric>::min(), std::numeric_limits<Numeric>::max(), Numeric(0), Numeric(1)});
}

void schema_tests()
{
    const auto &numeric = primitive_type<int32_t>();
    auto input = input_type(numeric);
    auto output = output_type(numeric);
    auto extra = input;
    extra.add_member(Member("extra", primitive_type<int32_t>()));
    rejects_schema(extra, output);
    rejects_schema(input_type(numeric, 3), output);
    rejects_schema(output, input);
    rejects_schema(StructType("Empty"), output);
    rejects_schema(numeric, output);
    rejects_schema(input, numeric);
    rejects_schema(input, output_type(numeric, SequenceType::UNBOUNDED));
    rejects_schema(input, output_type(primitive_type<uint32_t>()));
    rejects_schema(input, output_type(numeric, 3, "wrong_name"));
    rejects_schema(input_type(numeric, SequenceType::UNBOUNDED, "more_data"), output);
    rejects_schema(UnionType("UnionInput", numeric), output);
    rejects_schema(input, StructType("MissingFlag"));
    for (const auto &element : std::vector<DynamicType>{primitive_type<bool>(), primitive_type<char>(),
                primitive_type<wchar_t>(), StringType(16), WStringType(16),
                EnumType("Enumeration", {EnumMember("ZERO", 0)}),
                StructType("Nested"), SequenceType(numeric), ArrayType(numeric, 2)}) {
        rejects_schema(input_type(element), output_type(element));
    }
    StructType array_input("ArrayInput");
    array_input.add_member(Member("data", ArrayType(numeric, 3)));
    rejects_schema(array_input, output);
    array_input.add_member(Member("more_data", primitive_type<bool>()));
    rejects_schema(input, array_input);
    StructType wrong_flag("WrongFlag");
    wrong_flag.add_member(Member("data", SequenceType(numeric, 3)));
    wrong_flag.add_member(Member("more_data", numeric));
    rejects_schema(input, wrong_flag);
    auto extra_output = output;
    extra_output.add_member(Member("extra", numeric));
    rejects_schema(input, extra_output);
    for (bool optional : {false, true}) {
        StructType annotated("Annotated");
        Member member("data", SequenceType(numeric));
        if (optional) {
            member.optional(true);
        } else {
            member.key(true);
        }
        annotated.add_member(member);
        rejects_schema(annotated, output);
        for (bool annotate_flag : {false, true}) {
            StructType annotated_output("AnnotatedOutput");
            Member data("data", SequenceType(numeric, 3));
            Member flag("more_data", primitive_type<bool>());
            Member &target = annotate_flag ? flag : data;
            if (optional) {
                target.optional(true);
            } else {
                target.key(true);
            }
            annotated_output.add_member(data);
            annotated_output.add_member(flag);
            rejects_schema(input, annotated_output);
        }
    }
    StructType derived("Derived", input);
    rejects_schema(derived, output);
    rejects_schema(input, StructType("DerivedOutput", output));
    AliasType element_alias("NumberAlias", numeric);
    StructType aliased("Aliased");
    aliased.add_member(Member("data", AliasType("SequenceAlias", SequenceType(element_alias))));
    AliasType input_alias("InputAlias", aliased);
    AliasType output_alias("OutputAlias", output);
    Unbounded2BoundedTransformation transformation(type_info(input_alias), type_info(output_alias), {});
        DynamicData sample(input_alias);
        sample.set_values<DDS_Long>("data", {1, 2, 3, 4});
        SampleInfo info;
        info->native().valid_data = DDS_BOOLEAN_TRUE;
        std::vector<DynamicData *> samples;
        std::vector<SampleInfo *> infos;
        transformation.transform(samples, infos, {&sample}, {&info});
        check(samples.size() == 2 && samples[1]->get_values<DDS_Long>("data") == std::vector<DDS_Long>{4},
            "alias-backed conversion");
        transformation.return_loan(samples, infos);
}

void failure_tests()
{
    auto input = input_type(primitive_type<int32_t>());
    auto output = output_type(primitive_type<int32_t>());
    Unbounded2BoundedTransformation transformation(type_info(input), type_info(output), {});
    DynamicData sample(input);
    sample.set_values<int32_t>("data", {1, 2, 3, 4});
    SampleInfo valid;
    valid->native().valid_data = DDS_BOOLEAN_TRUE;
    SampleInfo invalid;
    std::vector<DynamicData *> samples;
    std::vector<SampleInfo *> infos;
    transformation.transform(samples, infos, {}, {});
    check(samples.empty() && infos.empty(), "empty callback");
    transformation.transform(samples, infos, {nullptr, &sample}, {&invalid, &valid});
    check(samples.size() == 2, "invalid data must be skipped");
    transformation.return_loan(samples, infos);
        DynamicData empty(input);
        transformation.transform(samples, infos, {&sample, &empty, &sample}, {&valid, &valid, &valid});
        check(samples.size() == 5 && !samples[2]->value<bool>("more_data")
                && samples[2]->get_values<DDS_Long>("data").empty(),
            "empty input between nonempty inputs");
        transformation.return_loan(samples, infos);
    rejects([&]() { transformation.transform(samples, infos, {&sample}, {}); }, "info count");
    rejects([&]() { transformation.transform(samples, infos, {&sample}, {nullptr}); }, "null info");
    rejects([&]() { transformation.transform(samples, infos, {&sample, nullptr}, {&valid, &valid}); },
            "rollback after staged chunks");
    check(samples.empty() && infos.empty(), "failed callback must not publish partial chunks");
    StructType incompatible("Incompatible");
    incompatible.add_member(Member("other", primitive_type<int32_t>()));
    DynamicData bad_sample(incompatible);
    rejects([&]() { transformation.transform(samples, infos, {&sample, &bad_sample}, {&valid, &valid}); },
            "conversion failure after staged chunks");
    check(samples.empty() && infos.empty(), "conversion failure rollback");
}

}

int main()
{
    try {
        integer_test<int16_t>();
        integer_test<uint16_t>();
        integer_test<int32_t>();
        integer_test<uint32_t>();
        integer_test<rti::core::int64>();
        integer_test<rti::core::uint64>();
        integer_test<uint8_t>();
#if RTICONNEXTDDS_HAS_INT8_TYPE
        integer_test<int8_t>();
        numeric_test<uint8_t>(primitive_type<rti::core::xtypes::octet_tag_t>(), {0, 128, 255});
#endif
        numeric_test<float>(primitive_type<float>(), {-0.0f, 0.5f, -3.25f,
                std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()});
        numeric_test<double>(primitive_type<double>(), {-0.0, 0.5, -3.25,
                std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()});
        rti::core::LongDouble long_double_value;
    #if RTI_CDR_LONG_DOUBLE_PRIMITIVE_TYPE
        long_double_value = 1.234567890123456789L;
    #else
        for (size_t byte_index = 0; byte_index < 16; ++byte_index) {
            long_double_value[byte_index] = static_cast<char>(byte_index * 7);
        }
    #endif
        numeric_test<rti::core::LongDouble>(primitive_type<rti::core::LongDouble>(),
            {rti::core::LongDouble(), long_double_value});
        schema_tests();
        failure_tests();
        std::cout << "Unbounded2Bounded direct tests passed" << std::endl;
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}