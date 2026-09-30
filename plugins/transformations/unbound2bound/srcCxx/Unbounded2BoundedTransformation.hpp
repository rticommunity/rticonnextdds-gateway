#ifndef UNBOUNDED2BOUNDED_TRANSFORMATION_HPP_
#define UNBOUNDED2BOUNDED_TRANSFORMATION_HPP_

#include <dds/dds.hpp>
#include <rti/routing/transf/TransformationPlugin.hpp>

class Unbounded2BoundedTransformation
        : public rti::routing::transf::DynamicDataTransformation {
public:
    Unbounded2BoundedTransformation(
            const rti::routing::TypeInfo &input_type,
            const rti::routing::TypeInfo &output_type,
            const rti::routing::PropertySet &properties);

    void transform(
            std::vector<dds::core::xtypes::DynamicData *> &output_samples,
            std::vector<dds::sub::SampleInfo *> &output_infos,
            const std::vector<dds::core::xtypes::DynamicData *> &input_samples,
            const std::vector<dds::sub::SampleInfo *> &input_infos) override;

    void return_loan(
            std::vector<dds::core::xtypes::DynamicData *> &samples,
            std::vector<dds::sub::SampleInfo *> &infos) override;

private:
    dds::core::xtypes::DynamicType output_type_;
    std::string member_name_;
    dds::core::xtypes::TypeKind element_kind_;
    uint32_t bound_;
};

class Unbounded2BoundedTransformationPlugin
        : public rti::routing::transf::TransformationPlugin {
public:
    explicit Unbounded2BoundedTransformationPlugin(
            const rti::routing::PropertySet &properties);

    rti::routing::transf::Transformation *create_transformation(
            const rti::routing::TypeInfo &input_type,
            const rti::routing::TypeInfo &output_type,
            const rti::routing::PropertySet &properties) override;

    void delete_transformation(
            rti::routing::transf::Transformation *transformation) override;
};

RTI_TRANSFORMATION_PLUGIN_CREATE_FUNCTION_DECL(Unbounded2BoundedTransformationPlugin)

#endif