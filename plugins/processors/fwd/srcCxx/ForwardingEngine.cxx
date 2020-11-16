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

#include <reda/reda_string.h>

#define RTI_PRCS_FWD_LOG_ARGS "rti::prcs::fwd::ForwardingEngine"

using namespace dds::core::xtypes;
using namespace rti::routing;
using namespace rti::routing::processor;
using namespace rti::routing::adapter;
using namespace rti::prcs::fwd;


InternalMatchingTableEntry::InternalMatchingTableEntry(
        const char *in_key,
        const char *out_name)
{
    this->in_key = in_key;
    this->out_name = out_name;

    RTI_PRCS_FWD_LOG_2(
            "new MATCH entry:",
            "in_key=%s, out_name=%s",
            in_key,
            out_name)
}

bool InternalMatchingTableEntry::match(const char *in_key)
{
    if (0 == REDAString_fnmatch(this->in_key.c_str(), in_key, 0)) {
        return true;
    } else {
        return false;
    }
}

InternalMatchingTableEntry &InternalMatchingTable::find(const char *in_key)
{
    for (InternalMatchingTableEntry &entry : entries) {
        if (entry.match(in_key)) {
            return entry;
        }
    }

    std::string in_key_str = in_key;
    throw dds::core::InvalidArgumentError(
            "no entry found for key: " + in_key_str);
}

InternalMatchingTableEntry &
        InternalMatchingTable::add(const char *in_key, const char *out_name)
{
    try {
        InternalMatchingTableEntry &cur_entry = find(in_key);
        cur_entry.out_name = out_name;
        return cur_entry;
    } catch (...) {
        InternalMatchingTableEntry entry(in_key, out_name);
        entries.push_back(entry);
        return entries.back();
    }
}


InternalMatchingTable
        InternalMatchingTable::from_matching_table(const MatchingTable &table)
{
    InternalMatchingTable internal_table;

    RTI_PRCS_FWD_LOG_FN(
            rti::prcs::fwd::InternalMatchingTable::from_matching_table)

    for (auto &entry : table.entries()) {
        internal_table.add(entry.in_key().c_str(), entry.out_name().c_str());
    }

    return internal_table;
}


void ForwardingEngine::on_data_available(Route &route)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ForwardingEngine::on_data_available)
    for (auto input : route.inputs<DynamicData>()) {
        RTI_PRCS_FWD_TRACE_1(
                "PROCESSING input:",
                "input=%s",
                input.name().c_str())

        try {
            LoanedSamples<DynamicData> samples = input->take();

            RTI_PRCS_FWD_TRACE_2(
                    "TAKEN samples:",
                    "input=%s, samples=%d",
                    input.name().c_str(),
                    samples.length())

            for (auto sample : samples) {
                if (sample.info().valid()) {
                    forward_data(route, input, sample.data());
                }
            }

        } catch (const std::exception &e) {
            RTI_PRCS_FWD_ERROR_2(
                    "EXCEPTION processing input:",
                    "input='%s', what='%s'",
                    input.name().c_str(),
                    e.what())
        }
    }
}

ForwardingEngine::ForwardingEngine(ForwardingEngineConfiguration &config)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ForwardingEngine::ForwardingEngine)

    this->fwd_table =
            InternalMatchingTable::from_matching_table(config.fwd_table());
}

void ForwardingEngine::forward_data(
        Route &route,
        TypedInput<dds::core::xtypes::DynamicData> &input,
        const dds::core::xtypes::DynamicData &data)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ForwardingEngine::forward_data)

    try {
        std::string fwd_key;
        get_forwarding_key(input, data, fwd_key);
        InternalMatchingTableEntry &fwd_entry = fwd_table.find(fwd_key.c_str());
        auto output = route.output<DynamicData>(fwd_entry.out_name);
        RTI_PRCS_FWD_LOG_4(
                "forwarding DATA:",
                "input=%s, key=%s, match=%s, out=%s",
                input.name().c_str(),
                fwd_key.c_str(),
                fwd_entry.in_key.c_str(),
                fwd_entry.out_name.c_str())
        output.write(data);
    } catch (const std::exception &e) {
        RTI_PRCS_FWD_ERROR_2(
                "EXCEPTION forwarding data:",
                "input='%s', what='%s'",
                input.name().c_str(),
                e.what())
    }
}