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
#ifndef STRUCT_ARRAY_TRANSF_HPP_
#define STRUCT_ARRAY_TRANSF_HPP_

#define SEQUENCE_MAX_SIZE 100

#include <dds/dds.hpp>

/**
 *  @class Sequence2ArrayTransformation
 *
 * @brief implementation of the Transformation.
 *
 * This transformation replaces sequences of any type by arrays.
 */
class Sequence2ArrayTransformation
        : public rti::routing::transf::DynamicDataTransformation {
public:
    Sequence2ArrayTransformation(
            const rti::routing::TypeInfo &input_type_info,
            const rti::routing::TypeInfo &output_type_info,
            const rti::routing::PropertySet &properties);

    void transform(
            std::vector<dds::core::xtypes::DynamicData *> &output_sample_seq,
            std::vector<dds::sub::SampleInfo *> &output_info_seq,
            const std::vector<dds::core::xtypes::DynamicData *>
                    &input_sample_seq,
            const std::vector<dds::sub::SampleInfo *> &input_info_seq);

    void return_loan(
            std::vector<dds::core::xtypes::DynamicData *> &sample_seq,
            std::vector<dds::sub::SampleInfo *> &info_seq);

private:
    void convert_sample(
            dds::core::xtypes::DynamicData & input_sample,
            dds::core::xtypes::DynamicData & output_sample);

    dds::core::xtypes::DynamicType input_type_info_;
    dds::core::xtypes::DynamicType output_type_info_;
};


/**
 *  @class Array2SequenceTransformation
 *
 * @brief implementation of the Transformation.
 *
 * This transformation replaces arrays of any type with sequences
 */
class Array2SequenceTransformation
        : public rti::routing::transf::DynamicDataTransformation {
public:
    Array2SequenceTransformation(
            const rti::routing::TypeInfo &input_type_info,
            const rti::routing::TypeInfo &output_type_info,
            const rti::routing::PropertySet &properties);

    void transform(
            std::vector<dds::core::xtypes::DynamicData *> &output_sample_seq,
            std::vector<dds::sub::SampleInfo *> &output_info_seq,
            const std::vector<dds::core::xtypes::DynamicData *>
                    &input_sample_seq,
            const std::vector<dds::sub::SampleInfo *> &input_info_seq);

    void return_loan(
            std::vector<dds::core::xtypes::DynamicData *> &sample_seq,
            std::vector<dds::sub::SampleInfo *> &info_seq);

private:
    void convert_sample(
            dds::core::xtypes::DynamicData & input_sample,
            dds::core::xtypes::DynamicData & output_sample);

    dds::core::xtypes::DynamicType input_type_info_;
    dds::core::xtypes::DynamicType output_type_info_;
};


template<typename T>
    static bool is_union_or_struct_type_compatible(
        const dds::core::xtypes::DynamicType & input_type,
        const dds::core::xtypes::DynamicType & output_type);

    static bool are_types_compatible(
        const dds::core::xtypes::DynamicType & input_type,
        const dds::core::xtypes::DynamicType & output_type);


/**
 * @class Sequence2ArrayTransformationPlugin
 *
 * @brief This class will be used by Routing Service to create and initialize
 * our custom Transformation Subclass. In this example, that class is
 * Sequence2ArrayTransformation.
 *
 * This class must use the macro
 * RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DECL(classname) in order to
 * create a C wrapper function that will be the dynamic library entry point
 * used by Routing Service.
 *
 */

class Sequence2ArrayTransformationPlugin
        : public rti::routing::transf::TransformationPlugin {
public:
    Sequence2ArrayTransformationPlugin(
            const rti::routing::PropertySet &properties);

    /*
     * @brief Creates an instance of Sequence2ArrayTransformation
     */
    rti::routing::transf::Transformation *create_transformation(
            const rti::routing::TypeInfo &input_type_info,
            const rti::routing::TypeInfo &output_type_info,
            const rti::routing::PropertySet &properties);

    void delete_transformation(
            rti::routing::transf::Transformation *transformation);
};

/**
 * @class Array2SequenceTransformationPlugin
 *
 * @brief This class will be used by Routing Service to create and initialize
 * our custom Transformation Subclass. In this example, that class is
 * Array2SequenceTransformation.
 *
 * This class must use the macro
 * RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DECL(classname) in order to
 * create a C wrapper function that will be the dynamic library entry point
 * used by Routing Service.
 *
 */

class Array2SequenceTransformationPlugin
        : public rti::routing::transf::TransformationPlugin {
public:
    Array2SequenceTransformationPlugin(
            const rti::routing::PropertySet &properties);

    /*
     * @brief Creates an instance of Sequence2ArrayTransformation
     */
    rti::routing::transf::Transformation *create_transformation(
            const rti::routing::TypeInfo &input_type_info,
            const rti::routing::TypeInfo &output_type_info,
            const rti::routing::PropertySet &properties);

    void delete_transformation(
            rti::routing::transf::Transformation *transformation);
};

/**
 * This macro defines a C-linkage symbol that can be used as create function
 * for plug-in registration through XML.
 *
 * The generated symbol has the name:
 *
 * \code
 * Sequence2ArrayTransformationPlugin_create_transformation_plugin
 * Array2SequenceTransformationPlugin_create_transformation_plugin
 * \endcode
 */
RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DECL(Sequence2ArrayTransformationPlugin)
RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DECL(Array2SequenceTransformationPlugin)

#endif /* STRUCT_ARRAY_TRANSF_HPP_ */
