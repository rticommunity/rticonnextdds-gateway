/*
 * (c) 2022 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */
#include <dds/dds.hpp>
#include <rti/routing/RoutingService.hpp>
#include <rti/routing/transf/TransformationPlugin.hpp>
#include <rti/core/xtypes/DynamicDataImpl.hpp>

#include "Seq2ArrayTransformation.hpp"
#include "DynamicDataHelpers.hpp"

using namespace dds::core::xtypes;
using dds::sub::SampleInfo;

/*
 * @brief Check that both types are compatible, a compatible type for this
 * transformation will mean that all the elements are the same but the sequences
 * in the input are replaced by arrays in the output.
 *
 */
bool Seq2ArrayTransformation::are_type_compatible(
        const dds::core::xtypes::DynamicType & input_type,
        const dds::core::xtypes::DynamicType & output_type)
{
    bool is_compatible = true;

    switch (input_type.kind().underlying()) {
    case TypeKind::STRUCTURE_TYPE: {
        if (output_type.kind() != TypeKind::STRUCTURE_TYPE) {
            std::string error("Error: the ouput field <" + output_type.name()
                    + "> is different to the input type <" + input_type.name());
            throw std::runtime_error(error);
        }

        auto input_struct_type = static_cast<const StructType &>(input_type);
        auto output_struct_type = static_cast<const StructType &>(output_type);

        if (input_struct_type.member_count() != output_struct_type.member_count()) {
            std::string error("Error: different member count of input and output "
                    "struct, input name: <" + input_struct_type.name()
                    + ">, output name <" + output_struct_type.name() + ">: "
                    + std::to_string(input_struct_type.member_count())
                    + " != " + std::to_string(output_struct_type.member_count()));
            throw std::runtime_error(error);
        }

        if (input_struct_type == output_struct_type) {
            return true;
        } else {
            // inspect element by element since it may be compatible
            // (output with arrays and input with sequences)
            for (auto member : input_struct_type.members()) {
                switch (member.type().kind().underlying()) {
                case TypeKind::STRUCTURE_TYPE:
                case TypeKind::SEQUENCE_TYPE:
                case TypeKind::ARRAY_TYPE:
                case TypeKind::UNION_TYPE:
                    is_compatible = is_compatible && are_type_compatible(
                            member.type(),
                            output_struct_type.member(member.name()).type());
                    if (!is_compatible) {
                        std::string error("Error: input member <"
                                + member.name().to_std_string()
                                + ">, not compatible with output name <"
                                + output_struct_type.member(member.name()).name().to_std_string()
                                + ">");
                        throw std::runtime_error(error);
                    }
                break;
                // primitive type
                default:
                    is_compatible = is_compatible
                            && (member.type() == output_struct_type.member(member.name()).type());
                    if (!is_compatible) {
                         std::string error("Error: input member <"
                                + member.name().to_std_string()
                                + ">, not compatible with output name <"
                                + output_struct_type.member(member.name()).name().to_std_string()
                                + ">");
                        throw std::runtime_error(error);
                    }
                    break;
                }

                if (!is_compatible) {
                    break;
                }
            }
        }

        break;
    }

    case TypeKind::UNION_TYPE: {
        if (output_type.kind() != TypeKind::UNION_TYPE) {
            std::string error("Error: the ouput field <" + output_type.name()
                    + "> is different to the input type <" + input_type.name());
            throw std::runtime_error(error);
        }

        auto input_union_type = static_cast<const UnionType &>(input_type);
        auto output_union_type = static_cast<const UnionType &>(output_type);

        if (input_union_type.member_count() != output_union_type.member_count()) {
            std::string error("Error: different member count of input and output "
                    "union, input name: <" + input_union_type.name()
                    + ">, output name <" + output_union_type.name() + ">: "
                    + std::to_string(input_union_type.member_count())
                    + " != " + std::to_string(output_union_type.member_count()));
            throw std::runtime_error(error);
        }

        if (input_union_type.discriminator() != output_union_type.discriminator()){
            std::string error("Error: different discriminator of input and output "
                    "unions. Input union <" + input_union_type.name()
                    + "> discriminator name <" + input_union_type.discriminator().name()
                    + "> is different of output union <" + output_union_type.name()
                    + "> discriminator name <" + output_union_type.discriminator().name()
                    + ">.");
            throw std::runtime_error(error);
        }

        if (input_union_type == output_union_type) {
            return true;
        } else {
            // inspect element by element since it may be compatible
            // (output with arrays and input with sequences)
            for (auto member : input_union_type.members()) {
                switch (member.type().kind().underlying()) {
                case TypeKind::STRUCTURE_TYPE:
                case TypeKind::SEQUENCE_TYPE:
                case TypeKind::ARRAY_TYPE:
                case TypeKind::UNION_TYPE:
                    is_compatible = is_compatible && are_type_compatible(
                            member.type(),
                            output_union_type.member(member.name()).type());
                    if (!is_compatible) {
                        std::string error("Error: input member <"
                                + member.name().to_std_string()
                                + ">, not compatible with output name <"
                                + output_union_type.member(member.name()).name().to_std_string()
                                + ">");
                        throw std::runtime_error(error);
                    }
                break;
                // primitive type
                default:
                    is_compatible = is_compatible
                            && (member.type() == output_union_type.member(member.name()).type());
                    if (!is_compatible) {
                        std::string error("Error: input member <"
                                + member.name().to_std_string()
                                + ">, not compatible with output name <"
                                + output_union_type.member(member.name()).name().to_std_string()
                                + ">");
                        throw std::runtime_error(error);
                    }
                    break;
                }

                if (!is_compatible) {
                    break;
                }
            }
        }
        break;
    }

    case TypeKind::ARRAY_TYPE: {
        if (output_type.kind() != TypeKind::ARRAY_TYPE) {
            std::string error("Error: the ouput field <" + output_type.name()
                    + "> is different to the input type <" + input_type.name());
            throw std::runtime_error(error);
        }

        auto input_array_type = static_cast<const ArrayType &>(input_type);
        auto output_array_type = static_cast<const ArrayType &>(output_type);

        if (input_array_type.dimension_count() != output_array_type.dimension_count()) {
            std::string error("Error: different dimension count of input and output "
                    "array, input name: <" + input_array_type.name()
                    + ">, output name <" + output_array_type.name() + ">: "
                    + std::to_string(input_array_type.dimension_count())
                    + " != " + std::to_string(output_array_type.dimension_count()));
            throw std::runtime_error(error);
        }

        if (input_array_type.total_element_count() != output_array_type.total_element_count()) {
            std::string error("Error: different total element count of input "
                    "and output array, input name: <" + input_array_type.name()
                    + ">, output name <" + output_array_type.name() + ">: "
                    + std::to_string(input_array_type.dimension_count())
                    + " != " + std::to_string(output_array_type.dimension_count()));
            throw std::runtime_error(error);
        }

        if (input_array_type.content_type() == output_array_type.content_type()) {
            return true;
        } else {
            // inspect element by element since it may be compatible
            // (output with arrays and input with sequences)
            switch (input_array_type.content_type().kind().underlying()) {
            case TypeKind::STRUCTURE_TYPE:
            case TypeKind::SEQUENCE_TYPE:
            case TypeKind::ARRAY_TYPE:
            case TypeKind::UNION_TYPE:
                is_compatible = is_compatible && are_type_compatible(
                        input_array_type.content_type(),
                        output_array_type.content_type());
                if (!is_compatible) {
                    std::string error("Error: content of input member <"
                            + input_array_type.name()
                            + ">, not compatible with content of output member <"
                            + output_array_type.name()
                            + ">");
                    throw std::runtime_error(error);
                }
                break;
            // primitive type
            default:
                // not needed because if it is a primitive content_type, this
                // has been already checked in the previous if condition
                break;
            }
        }
        break;
    }

    case TypeKind::SEQUENCE_TYPE: {
        if (output_type.kind() != TypeKind::ARRAY_TYPE) {
            std::string error("Error: the ouput field <" + output_type.name()
                    + "> is not an array.");
            throw std::runtime_error(error);
        }

        auto input_sequence_type = static_cast<const SequenceType &>(input_type);
        auto output_array_type = static_cast<const ArrayType &>(output_type);

        if (output_array_type.dimension_count() != 1) {
            std::string error("Error: cannot transform a sequence to a "
                    "multidimensional array. The array dimension count should be 1.");
            throw std::runtime_error(error);
        }

        if (input_sequence_type.content_type() == output_array_type.content_type()) {
            return true;
        } else {
            // inspect element by element since it may be compatible
            // (output with arrays and input with sequences)
            switch (input_sequence_type.content_type().kind().underlying()) {
                case TypeKind::STRUCTURE_TYPE:
                case TypeKind::SEQUENCE_TYPE:
                case TypeKind::ARRAY_TYPE:
                case TypeKind::UNION_TYPE:
                    is_compatible = is_compatible && are_type_compatible(
                            input_sequence_type.content_type(),
                            output_array_type.content_type());
                    if (!is_compatible) {
                        std::string error("Error: content of input member <"
                                + input_sequence_type.name()
                                + ">, not compatible with content of output member <"
                                + output_array_type.name()
                                + ">");
                        throw std::runtime_error(error);
                    }
                break;
                // primitive type
                default:
                    // not needed because if it is a primitive content_type, this
                    // has been already checked in the previous if condition
                    break;
                }
            }
            break;
        }
    default:
        std::string error("Error: incompatible type input <" + input_type.name()
                + ">, and output <" + output_type.name() + ">");
        throw std::runtime_error(error);
        break;
    }

    return is_compatible;
}

Seq2ArrayTransformation::Seq2ArrayTransformation(
        const rti::routing::TypeInfo &input_type_info,
        const rti::routing::TypeInfo &output_type_info,
        const rti::routing::PropertySet &properties)
        : input_type_info_(input_type_info.dynamic_type()),
          output_type_info_(output_type_info.dynamic_type())
{
    if (!are_type_compatible(input_type_info_, output_type_info_)) {
        throw std::runtime_error("Error: input and ouput types are not compatible");
    }
    // properties are not used because there is no additional configuration for
    // this transformation
}

/*
 * @brief Substitutes sequences by arrays of any type.
 *
 */
void Seq2ArrayTransformation::convert_sample(
        DynamicData &input_sample,
        DynamicData &output_sample)
{
    // convert all the elements within a struct, sequence, array or union.
    for (uint32_t i = 1; i <= input_sample.member_count(); ++i) {
        uint32_t member_to_process = i;
        bool is_union_member = false;

        // unions only have to check the value that the discriminator identifies
        if (input_sample.type_kind() == TypeKind::UNION_TYPE) {
            member_to_process = input_sample.discriminator_value();
            is_union_member = true;
        }

        if (!input_sample.member_exists(member_to_process)) {
            continue;
        }

        // convert a member of the main Dynamic Data
        switch (input_sample.member_info(member_to_process).member_kind().underlying()) {
        case TypeKind::SEQUENCE_TYPE:
        case TypeKind::ARRAY_TYPE: {
            auto input_loaned_member = input_sample.loan_value(member_to_process);
            auto output_loaned_member = output_sample.loan_value(member_to_process);

            // in case of input sequences, check that the size of output array
            // is enough to store all the values the sequence has.

            // Get the size of the ouput array
            const ArrayType &array_type =
                    static_cast<const ArrayType &>(output_loaned_member.get().type());

            // Check that the size of the ouput array is enough
            if (input_loaned_member.get().member_count() > array_type.total_element_count()) {
                std::string error("Error: not enough space to copy output field <"
                        + output_loaned_member.get().type().name()
                        + "> (max size <"
                        + std::to_string(array_type.total_element_count())
                        + ">) into input field <"
                        + input_loaned_member.get().type().name()
                        + "> (actual size <"
                        + std::to_string(input_loaned_member.get().member_count())
                        + ">).");
                throw std::runtime_error(error);
            }

            // if it is a sequence of structs, sequences, arrays or unions,
            // the inner type may not be the same, hence, no direct copy may be
            // perform
            switch (input_loaned_member.get().member_info(1).member_kind().underlying()) {
            case TypeKind::STRUCTURE_TYPE:
            case TypeKind::SEQUENCE_TYPE:
            case TypeKind::ARRAY_TYPE:
            case TypeKind::UNION_TYPE: {
                for (uint32_t j = 1; j <= input_loaned_member.get().member_count(); ++j) {
                    auto input_loaned_element = input_loaned_member.get().loan_value(j);
                    auto output_loaned_element = output_loaned_member.get().loan_value(j);
                    convert_sample(input_loaned_element.get(), output_loaned_element.get());

                    input_loaned_element.return_loan();
                    output_loaned_element.return_loan();
                }
                break;
            }
            default:
                // if it is a sequence of primitive types (including enums and
                // strings), do a direct copy
                input_loaned_member.return_loan();
                output_loaned_member.return_loan();
                rti::common::dynamic_data::copy_primitive_array_elements(
                         input_sample,
                         output_sample,
                         member_to_process,
                         output_sample.member_info(member_to_process).element_count());
                break;

            }
            input_loaned_member.return_loan();
            output_loaned_member.return_loan();
            break;
        }
        case TypeKind::STRUCTURE_TYPE:
        case TypeKind::UNION_TYPE: {
            auto input_loaned_member = input_sample.loan_value(member_to_process);
            auto output_loaned_member = output_sample.loan_value(member_to_process);
            // call recursively the convert_sample function since the struct or
            // union may contain sequences
            convert_sample(input_loaned_member.get(), output_loaned_member.get());
            input_loaned_member.return_loan();
            output_loaned_member.return_loan();
            break;
        }
        default:
            // copy directly the primitive member (including strings and enums)
            rti::common::dynamic_data::copy_primitive_member(
                    input_sample,
                    output_sample,
                    member_to_process);
        }
        if (is_union_member) {
            // break the loop because unions only have to process the member
            // that the discriminator is "pointing" to
            break;
        }
    }
}

void Seq2ArrayTransformation::transform(
        std::vector<DynamicData *> &output_sample_seq,
        std::vector<SampleInfo *> &output_info_seq,
        const std::vector<DynamicData *> &input_sample_seq,
        const std::vector<SampleInfo *> &input_info_seq)
{
    // resize the output sample and info sequences to hold as many samples
    // as the input sequences
    output_sample_seq.resize(input_sample_seq.size());
    output_info_seq.resize(input_info_seq.size());

    // Convert each individual input sample
    for (size_t i = 0; i < input_sample_seq.size(); ++i) {
        // convert data
        output_sample_seq[i] = new DynamicData(output_type_info_);
        convert_sample(*input_sample_seq[i], *output_sample_seq[i]);

        // copy info as is
        output_info_seq[i] = new SampleInfo(*input_info_seq[i]);
    }
}

void Seq2ArrayTransformation::return_loan(
        std::vector<DynamicData *> &sample_seq,
        std::vector<SampleInfo *> &info_seq)
{
    for (size_t i = 0; i < sample_seq.size(); ++i) {
        delete sample_seq[i];
        delete info_seq[i];
    }
}


/*
 * --- Seq2ArrayTransformationPlugin ----------------------------------------
 */
Seq2ArrayTransformationPlugin::Seq2ArrayTransformationPlugin(
        const rti::routing::PropertySet &)
{
    // no configuration properties for this plug-in
}

rti::routing::transf::Transformation *Seq2ArrayTransformationPlugin::
        create_transformation(
                const rti::routing::TypeInfo &input_type_info,
                const rti::routing::TypeInfo &output_type_info,
                const rti::routing::PropertySet &properties)
{
    return new Seq2ArrayTransformation(
            input_type_info,
            output_type_info,
            properties);
}

void Seq2ArrayTransformationPlugin::delete_transformation(
        rti::routing::transf::Transformation *transformation)
{
    delete transformation;
}


RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DEF(Seq2ArrayTransformationPlugin);