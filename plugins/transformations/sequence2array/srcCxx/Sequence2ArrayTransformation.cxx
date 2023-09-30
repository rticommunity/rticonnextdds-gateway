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

#include "Sequence2ArrayTransformation.hpp"
#include "DynamicDataHelpers.hpp"

using namespace dds::core::xtypes;
using dds::sub::SampleInfo;

/*
 * @brief Check that both types are compatible. This function only works with
 * unions and structs. A compatible type for this transformation will mean that
 * all the elements are the same but the sequences in the input are replaced by
 * arrays in the output.
 *
 * This function will indirectly call itself (by means of a call to
 * `Sequence2ArrayTransformation::are_types_compatible()`) in order to:
 * - Validate the compatibility of two members of a structured type
 *   (i.e. union or struct).
 * - Validate the compatibility of the element type of two members of a
 *   collection type (i.e. array or sequence).
 *
 * This function should only be called from within
 * `Sequence2ArrayTransformation::are_types_compatible()`.
 */
template<typename T>
bool is_union_or_struct_type_compatible(
        const dds::core::xtypes::DynamicType & input_type,
        const dds::core::xtypes::DynamicType & output_type)
{
    // Check that the types are the same
    if (input_type.kind() != output_type.kind()) {
        rti::routing::Logger::instance().error("incompatible type kinds: "
                + input_type.name() + ", "
                + output_type.name());
        return true;
    }

    // check that the types are either union or struct
    if (input_type.kind() != TypeKind::UNION_TYPE
            && input_type.kind() != TypeKind::STRUCTURE_TYPE)  {
        rti::routing::Logger::instance().error("expected union or struct, found: "
                + input_type.name() + " "
                + std::to_string(input_type.kind().underlying()));
        return false;
    }

    auto t_input_type = static_cast<const T &>(input_type);
    auto t_output_type = static_cast<const T &>(output_type);
    bool is_compatible = true;

    // in case it is a union, check that the discriminator is the same.
    // We rely on the overloaded `operator==` provided by DynamicType.
    if (input_type.kind() == TypeKind::UNION_TYPE) {
        auto union_input_type = static_cast<const UnionType &>(input_type);
        auto union_output_type = static_cast<const UnionType &>(output_type);
        if (union_input_type.discriminator() != union_output_type.discriminator()){
            rti::routing::Logger::instance().error(
                    "incompatible union discriminators: "
                    + union_input_type.name()
                    + "(" + union_input_type.discriminator().name() + "), "
                    + union_output_type.name()
                    + "(" + union_output_type.discriminator().name() + ")");
            return false;
        }
    }

    // check that the number of elements are the same
    if (t_input_type.member_count() != t_output_type.member_count()) {
       rti::routing::Logger::instance().error(
                "incompatible members count: "
                + t_input_type.name()
                + "(" + std::to_string(t_input_type.member_count()) + "), "
                + t_output_type.name()
                + "(" + std::to_string(t_output_type.member_count()) + ")");
        return false;
    }

    if (t_input_type == t_output_type) {
        return true;
    } else {
        // inspect member by member since it may be compatible
        // (output with arrays and input with sequences)
        for (auto input_member : t_input_type.members()) {
            auto output_member = t_output_type.member(input_member.name());

            // Resolve alias before going into the switch
            auto input_member_type = rti::core::xtypes::resolve_alias(input_member.type());
            auto output_member_type = rti::core::xtypes::resolve_alias(output_member.type());

            switch (input_member_type.kind().underlying()) {
            case TypeKind::STRUCTURE_TYPE:
            case TypeKind::SEQUENCE_TYPE:
            case TypeKind::ARRAY_TYPE:
            case TypeKind::UNION_TYPE:
                // indirect recursion with the function are_types_compatible
                is_compatible = is_compatible && are_types_compatible(
                        input_member_type,
                        output_member_type);
                if (!is_compatible) {
                    rti::routing::Logger::instance().error(
                            "incompatible members: "
                            + input_member.name().to_std_string()
                            + ", "
                            + output_member.name().to_std_string());
                    return false;
                }
                break;
            // primitive type
            default:
                is_compatible = is_compatible
                        && (input_member_type == output_member_type);
                if (!is_compatible) {
                   rti::routing::Logger::instance().error(
                            "incompatible members: "
                            + input_member.name().to_std_string()
                            + ", "
                            + output_member.name().to_std_string());
                    return false;
                }
                break;
            }

            if (!is_compatible) {
                break;
            }
        }
    }
    return is_compatible;
}

/*
 * @brief Check that both types are compatible, a compatible type for this
 * transformation will mean that all the elements are the same but the sequences
 * in the input are replaced by arrays in the output.
 *
 */
bool are_types_compatible(
        const dds::core::xtypes::DynamicType & input_type,
        const dds::core::xtypes::DynamicType & output_type)
{
    bool is_compatible = true;

    switch (input_type.kind().underlying()) {
    case TypeKind::STRUCTURE_TYPE: {
        is_compatible = is_union_or_struct_type_compatible<StructType>(
                input_type, output_type);
        break;
    }

    case TypeKind::UNION_TYPE: {
        is_compatible = is_union_or_struct_type_compatible<UnionType>(
                input_type, output_type);
        break;
    }

    case TypeKind::ARRAY_TYPE: {
        if (output_type.kind() != TypeKind::ARRAY_TYPE && output_type.kind() != TypeKind::SEQUENCE_TYPE) {
            rti::routing::Logger::instance().error("output is not an array or sequence");
            return false;
        }

        auto input_array_type = static_cast<const ArrayType &>(input_type);
        // ARRAY -> ARRAY
        if (output_type.kind() == TypeKind::ARRAY_TYPE) {
            auto output_array_type = static_cast<const ArrayType &>(output_type);

            if (input_array_type.dimension_count() != output_array_type.dimension_count()) {
                rti::routing::Logger::instance().error("different array dimension count: "
                        + std::to_string(input_array_type.dimension_count())
                        + " != " + std::to_string(output_array_type.dimension_count()));
                return false;
            }

            if (input_array_type.total_element_count() != output_array_type.total_element_count()) {
                rti::routing::Logger::instance().error("different array total element count: "
                        + std::to_string(input_array_type.total_element_count())
                        + " != " + std::to_string(output_array_type.total_element_count()));
                return false;
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
                    is_compatible = is_compatible && are_types_compatible(
                            input_array_type.content_type(),
                            output_array_type.content_type());
                    if (!is_compatible) {
                        rti::routing::Logger::instance().error(
                                "incompatible array content.");
                        return false;
                    }
                    break;
                // primitive type
                default:
                    // this means two arrays of different primitive types
                    is_compatible = false;
                    break;
                }
            }
        // ARRAY -> SEQUENCE
        } else { // TypeKind::SEQUENCE_TYPE
            auto output_sequence_type = static_cast<const SequenceType &>(output_type);

            if (input_array_type.total_element_count() > output_sequence_type.bounds()) {
                rti::routing::Logger::instance().error("array total element count: "
                        + std::to_string(input_array_type.total_element_count())
                        + " > " + std::to_string(output_sequence_type.bounds()));
                return false;
            }
            if (input_array_type.content_type() == output_sequence_type.content_type()) {
                return true;
            } else {
                // inspect element by element since it may be compatible
                switch (input_array_type.content_type().kind().underlying()) {
                case TypeKind::STRUCTURE_TYPE:
                case TypeKind::SEQUENCE_TYPE:
                case TypeKind::ARRAY_TYPE:
                case TypeKind::UNION_TYPE:
                    is_compatible = is_compatible && are_types_compatible(
                            input_array_type.content_type(),
                            output_sequence_type.content_type());
                    if (!is_compatible) {
                        rti::routing::Logger::instance().error("incompatible array content.");
                        return false;
                    }
                    break;
                // primitive type
                default:
                    is_compatible = false;
                    break;
                }
            }
        }
        break;
    }

    case TypeKind::SEQUENCE_TYPE: {
        if (output_type.kind() != TypeKind::ARRAY_TYPE && output_type.kind() != TypeKind::SEQUENCE_TYPE) {
            rti::routing::Logger::instance().error(
                    "the output element is not an array or a sequence");
            return false;
        }

        auto input_sequence_type = static_cast<const SequenceType &>(input_type);
        // SEQUENCE -> ARRAY
        if (output_type.kind() == TypeKind::ARRAY_TYPE) {
            auto output_array_type = static_cast<const ArrayType &>(output_type);

            if (output_array_type.dimension_count() != 1) {
                rti::routing::Logger::instance().error("cannot transform a sequence "
                        "to a multidimensional array. The array dimension count "
                        "should be 1.");
                return false;
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
                        is_compatible = is_compatible && are_types_compatible(
                                input_sequence_type.content_type(),
                                output_array_type.content_type());
                        if (!is_compatible) {
                            rti::routing::Logger::instance().error(
                                    "incompatible array and sequence content.");
                            return false;
                        }
                    break;
                    // primitive type
                    default:
                        // not needed because if it is a primitive content_type, this
                        // has been already checked in the previous if condition
                        break;
                    }
            }
        // SEQUENCE -> SEQUENCE
        } else {  // TypeKind::SEQUENCE_TYPE
            auto output_sequence_type = static_cast<const SequenceType &>(output_type);
            
            if (input_sequence_type.content_type() == output_sequence_type.content_type()) {
                return true;
            } else {
                // inspect element by element since it may be compatible
                switch (input_sequence_type.content_type().kind().underlying()) {
                    case TypeKind::STRUCTURE_TYPE:
                    case TypeKind::SEQUENCE_TYPE:
                    case TypeKind::ARRAY_TYPE:
                    case TypeKind::UNION_TYPE:
                        is_compatible = is_compatible && are_types_compatible(
                                input_sequence_type.content_type(),
                                output_sequence_type.content_type());
                        if (!is_compatible) {
                            rti::routing::Logger::instance().error("incompatible sequence content.");
                            return false;
                        }
                    break;
                    // primitive type
                    default:
                        // not needed because if it is a primitive content_type, this
                        // has been already checked in the previous if condition
                        break;
                    }
            }
        }
        break;
    }
    default:
        rti::routing::Logger::instance().error("incompatible type input.");
        return false;
    }

    return is_compatible;
}


Sequence2ArrayTransformation::Sequence2ArrayTransformation(
        const rti::routing::TypeInfo &input_type_info,
        const rti::routing::TypeInfo &output_type_info,
        const rti::routing::PropertySet &properties)
        : input_type_info_(input_type_info.dynamic_type()),
          output_type_info_(output_type_info.dynamic_type())
{
    if (!are_types_compatible(input_type_info_, output_type_info_)) {
        throw std::runtime_error("input and ouput types are not compatible.");
    }
    // properties are not used because there is no additional configuration for
    // this transformation
}

Array2SequenceTransformation::Array2SequenceTransformation(
        const rti::routing::TypeInfo &input_type_info,
        const rti::routing::TypeInfo &output_type_info,
        const rti::routing::PropertySet &properties)
        : input_type_info_(input_type_info.dynamic_type()),
          output_type_info_(output_type_info.dynamic_type())
{
    if (!are_types_compatible(input_type_info_, output_type_info_)) {
        throw std::runtime_error("input and output types are not compatible.");
    }
    // properties are not used because there is no additional configuration for
    // this transformation
}


/*
 * @brief Substitutes sequences by arrays of any type.
 *
 */
void Sequence2ArrayTransformation::convert_sample(
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
                std::string error("not enough space to copy output field");
                        // CAN'T GET TYPE NAME ERROR
                        /*
                        + output_loaned_member.get().type().name()
                        + "> (max size <"
                        + std::to_string(array_type.total_element_count())
                        + ">) into input field <"
                        + input_loaned_member.get().type().name()
                        + "> (actual size <"
                        + std::to_string(input_loaned_member.get().member_count())
                        + ">).");
                        */
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

/*
 * @brief Substitutes sequences by arrays of any type.
 *
 */
void Array2SequenceTransformation::convert_sample(
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

            // in case of input arrays, check that the size of output sequence
            // is enough to store all the values the array has

            // Get the size of the ouput sequence
            const SequenceType &sequence_type =
                    static_cast<const SequenceType &>(output_loaned_member.get().type());

            // Check that the bounds of the ouput sequence is enough
            if (input_loaned_member.get().member_count() > sequence_type.bounds()) {
                std::string error("not enough space to copy output field");
                        // CAN'T GET TYPE NAME ERROR
                        /*
                        + output_loaned_member.get().type().name()
                        + "> (max size <"
                        + std::to_string(sequence_type.bounds())
                        + ">) into input field <"
                        + input_loaned_member.get().type().name()
                        + "> (actual size <"
                        + std::to_string(input_loaned_member.get().member_count())
                        + ">).");
                        */
                throw std::runtime_error(error);
            }


            // if it is an array of structs, sequences, arrays or unions,
            // the inner type may not be the same, hence, no direct copy may be performed
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
                // if it is an array of primitive types (including enums and
                // strings), do a direct copy
                input_loaned_member.return_loan();
                output_loaned_member.return_loan();
                rti::common::dynamic_data::copy_primitive_sequence_elements(
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
            // union may contain arrays
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


void Sequence2ArrayTransformation::transform(
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

void Array2SequenceTransformation::transform(
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


void Sequence2ArrayTransformation::return_loan(
        std::vector<DynamicData *> &sample_seq,
        std::vector<SampleInfo *> &info_seq)
{
    for (size_t i = 0; i < sample_seq.size(); ++i) {
        delete sample_seq[i];
        delete info_seq[i];
    }
}

void Array2SequenceTransformation::return_loan(
        std::vector<DynamicData *> &sample_seq,
        std::vector<SampleInfo *> &info_seq)
{
    for (size_t i = 0; i < sample_seq.size(); ++i) {
        delete sample_seq[i];
        delete info_seq[i];
    }
}



/*
 * --- Sequence2ArrayTransformationPlugin --------------------------------------
 */
Sequence2ArrayTransformationPlugin::Sequence2ArrayTransformationPlugin(
        const rti::routing::PropertySet &)
{
    // no configuration properties for this plug-in
}

rti::routing::transf::Transformation *Sequence2ArrayTransformationPlugin::
        create_transformation(
                const rti::routing::TypeInfo &input_type_info,
                const rti::routing::TypeInfo &output_type_info,
                const rti::routing::PropertySet &properties)
{
    return new Sequence2ArrayTransformation(
            input_type_info,
            output_type_info,
            properties);
}

void Sequence2ArrayTransformationPlugin::delete_transformation(
        rti::routing::transf::Transformation *transformation)
{
    delete transformation;
}

/*
 * --- Array2SequenceTransformationPlugin --------------------------------------
 */
Array2SequenceTransformationPlugin::Array2SequenceTransformationPlugin(
        const rti::routing::PropertySet &)
{
    // no configuration properties for this plug-in
}

rti::routing::transf::Transformation *Array2SequenceTransformationPlugin::
        create_transformation(
                const rti::routing::TypeInfo &input_type_info,
                const rti::routing::TypeInfo &output_type_info,
                const rti::routing::PropertySet &properties)
{
    return new Array2SequenceTransformation(
            input_type_info,
            output_type_info,
            properties);
}

void Array2SequenceTransformationPlugin::delete_transformation(
        rti::routing::transf::Transformation *transformation)
{
    delete transformation;
}


RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DEF(Sequence2ArrayTransformationPlugin);
RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DEF(Array2SequenceTransformationPlugin);