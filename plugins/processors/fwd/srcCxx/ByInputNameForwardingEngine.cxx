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

#include <rtiprocess_fwd.hpp>

#define RTI_PRCS_FWD_LOG_ARGS "rti::prcs::fwd::ByInputNameForwardingEngine"

using namespace dds::core::xtypes;
using namespace rti::routing;
using namespace rti::routing::processor;
using namespace rti::routing::adapter;
using namespace rti::prcs::fwd;

ByInputNameForwardingEngine::ByInputNameForwardingEngine(
        ByInputNameForwardingEngineConfiguration &config)
        : ForwardingEngine(config)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ByInputNameForwardingEngine::
                                ByInputNameForwardingEngine)
}

void ByInputNameForwardingEngine::get_forwarding_key(
        TypedInput<DynamicData> &input,
        const DynamicData &data,
        std::string &fwd_key_out)
{
    RTI_PRCS_FWD_LOG_FN(
            rti::prcs::fwd::ByInputNameForwardingEngine::get_forwarding_key)

    fwd_key_out = input.name();

    RTI_PRCS_FWD_TRACE_3(
            "forwarding KEY:",
            "input=%s, data=%p, key=%s",
            input.name().c_str(),
            data,
            fwd_key_out.c_str())
}


Processor *ByInputNameForwardingEnginePlugin::create_processor(
        Route &route,
        const PropertySet &properties)
{
    RTI_PRCS_FWD_LOG_FN(
            rti::prcs::fwd::ByInputNameForwardingEnginePlugin::create_processor)

    ByInputNameForwardingEngineConfiguration config;

    property::parse_config(properties, config);

    return new ByInputNameForwardingEngine(config);
}

void ByInputNameForwardingEnginePlugin::delete_processor(
        rti::routing::processor::Route &route,
        rti::routing::processor::Processor *processor)
{
    RTI_PRCS_FWD_LOG_FN(
            rti::prcs::fwd::ByInputNameForwardingEnginePlugin::delete_processor)
    delete processor;
}

ByInputNameForwardingEnginePlugin::ByInputNameForwardingEnginePlugin(
        const rti::routing::PropertySet &)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ByInputNameForwardingEnginePlugin::
                                ByInputNameForwardingEnginePlugin)
}

struct RTI_RoutingServiceProcessorPlugin *
        RTI_PRCS_FWD_ByInputNameForwardingEnginePlugin_create(
                const struct RTI_RoutingServiceProperties *native_properties,
                RTI_RoutingServiceEnvironment *environment)
{
    RTI_PRCS_FWD_LOG_FN(RTI_PRCS_FWD_ByInputNameForwardingEnginePlugin_create)

    PropertySet properties;
    rti::routing::PropertyAdapter::add_properties_from_native(
            properties,
            native_properties);
    try {
        return rti::routing::processor::detail::ProcessorPluginForwarder::
                create_plugin(
                        new ByInputNameForwardingEnginePlugin(properties));
    } catch (const std::exception &ex) {
        RTI_RoutingServiceEnvironment_set_error(environment, "%s", ex.what());
    } catch (...) {
    }

    return NULL;
}