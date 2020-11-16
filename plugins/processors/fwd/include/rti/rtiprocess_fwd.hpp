/******************************************************************************/
/* (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
/*                                                                            */
/* RTI grants Licensee a license to use, modify, compile, and create          */
/* derivative works of the software solely for use with RTI Connext DDS.      */
/* Licensee may redistribute copies of the software provided that all such    */
/* copies are subject to this license.                                        */
/* The software is provided "as is", with no warranty of any type, including  */
/* any warranty for fitness for any purpose. RTI is under no obligation to    */
/* maintain or support the software.  RTI shall not be liable for any         */
/* incidental or consequential damages arising out of the use or inability to */
/* use the software.                                                          */
/*                                                                            */
/******************************************************************************/

#ifndef rtiprocess_fwd_hpp
#define rtiprocess_fwd_hpp

#include <rtiprocess_fwd_log.hpp>
#include <rtiprocess_fwd_platform.hpp>
#include <rtiprocess_fwd_properties.hpp>


namespace rti { namespace prcs { namespace fwd {

struct InternalMatchingTableEntry {
    bool match(const char *in_key);

    InternalMatchingTableEntry(const char *in_key, const char *out_name);

    std::string in_key;
    std::string out_name;
};

struct InternalMatchingTable {
    InternalMatchingTableEntry &find(const char *in_key);
    InternalMatchingTableEntry &add(const char *in_key, const char *out_name);

    std::vector<InternalMatchingTableEntry> entries;

    static InternalMatchingTable
            from_matching_table(const fwd::MatchingTable &table);
};

class ForwardingEngine : public rti::routing::processor::NoOpProcessor {
public:
    void on_data_available(rti::routing::processor::Route &);

    ForwardingEngine(ForwardingEngineConfiguration &config);

protected:
    InternalMatchingTable fwd_table;

    void forward_data(
            rti::routing::processor::Route &route,
            rti::routing::processor::TypedInput<dds::core::xtypes::DynamicData>
                    &input,
            const dds::core::xtypes::DynamicData &data);

    virtual void get_forwarding_key(
            rti::routing::processor::TypedInput<dds::core::xtypes::DynamicData>
                    &input,
            const dds::core::xtypes::DynamicData &data,
            std::string &fwd_key_out) = 0;
};

class ByInputNameForwardingEngine : public ForwardingEngine {
public:
    ByInputNameForwardingEngine(
            ByInputNameForwardingEngineConfiguration &config);

protected:
    void get_forwarding_key(
            rti::routing::processor::TypedInput<dds::core::xtypes::DynamicData>
                    &input,
            const dds::core::xtypes::DynamicData &data,
            std::string &fwd_key_out);
};


struct InputMemberValue {
    static const char *FORMAT_BOOLEAN;
    static const char *FORMAT_UINT8;
    static const char *FORMAT_UINT16;
    static const char *FORMAT_UINT32;
    static const char *FORMAT_UINT64;
    static const char *FORMAT_CHAR8;
    static const char *FORMAT_INT16;
    static const char *FORMAT_INT32;
    static const char *FORMAT_INT64;
    static const char *FORMAT_FLOAT32;
    static const char *FORMAT_FLOAT64;
    static const char *FORMAT_STRING;
    std::string name;
    dds::core::optional<rti::core::xtypes::DynamicDataMemberInfo> info;
    std::string string_format;

    InputMemberValue();

    InputMemberValue(
            const char *name,
            const dds::core::xtypes::DynamicData &data);

    void to_string(
            const dds::core::xtypes::DynamicData &data,
            std::string &str_out);
};

class ByInputValueForwardingEngine : public ForwardingEngine {
public:
    ByInputValueForwardingEngine(
            ByInputValueForwardingEngineConfiguration &config);

protected:
    std::map<std::string, std::map<std::string, InputMemberValue>>
            input_members_cache;
    InternalMatchingTable input_members;

    void get_forwarding_key(
            rti::routing::processor::TypedInput<dds::core::xtypes::DynamicData>
                    &input,
            const dds::core::xtypes::DynamicData &data,
            std::string &fwd_key_out);

    InputMemberValue &cache_input_member(
            rti::routing::processor::TypedInput<dds::core::xtypes::DynamicData>
                    &input,
            const dds::core::xtypes::DynamicData &data);

    std::map<std::string, InputMemberValue> &get_input_map(
            rti::routing::processor::TypedInput<dds::core::xtypes::DynamicData>
                    &input);
};

class ByInputNameForwardingEnginePlugin
        : public rti::routing::processor::ProcessorPlugin {
public:
    rti::routing::processor::Processor *create_processor(
            rti::routing::processor::Route &route,
            const rti::routing::PropertySet &properties) override;
    void delete_processor(
            rti::routing::processor::Route &route,
            rti::routing::processor::Processor *processor) override;

    ByInputNameForwardingEnginePlugin(const rti::routing::PropertySet &);
};

class ByInputValueForwardingEnginePlugin
        : public rti::routing::processor::ProcessorPlugin {
public:
    rti::routing::processor::Processor *create_processor(
            rti::routing::processor::Route &route,
            const rti::routing::PropertySet &properties) override;
    void delete_processor(
            rti::routing::processor::Route &route,
            rti::routing::processor::Processor *processor) override;

    ByInputValueForwardingEnginePlugin(const rti::routing::PropertySet &);
};

}}}  // namespace rti::prcs::fwd

extern "C" RTI_USER_DLL_EXPORT struct RTI_RoutingServiceProcessorPlugin *
        RTI_PRCS_FWD_ByInputNameForwardingEnginePlugin_create(
                const struct RTI_RoutingServiceProperties *native_properties,
                RTI_RoutingServiceEnvironment *environment);

extern "C" RTI_USER_DLL_EXPORT struct RTI_RoutingServiceProcessorPlugin *
        RTI_PRCS_FWD_ByInputValueForwardingEnginePlugin_create(
                const struct RTI_RoutingServiceProperties *native_properties,
                RTI_RoutingServiceEnvironment *environment);

#endif /* rtiprocess_fwd_hpp */