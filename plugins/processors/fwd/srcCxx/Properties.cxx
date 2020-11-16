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

#include <json.h>
#include <rtiprocess_fwd.hpp>

#define RTI_PRCS_FWD_LOG_ARGS "rti::prcs::fwd::properties"

using namespace rti::prcs::fwd;
using namespace dds::core::xtypes;
using namespace rti::routing;
using namespace rti::routing::processor;
using namespace rti::routing::adapter;

const std::string property::PREFIX = "";
const std::string property::FORWARDING_TABLE =
        property::PREFIX + "forwarding_table";

const std::string property::INPUT_MEMBERS_TABLE =
        property::PREFIX + "input_members";

const std::string property::FORWARDING_TABLE_KEY_IN_KEY = "input";
const std::string property::FORWARDING_TABLE_KEY_OUT_NAME = "output";

const std::string property::INPUT_MEMBERS_TABLE_KEY_IN_KEY = "input";
const std::string property::INPUT_MEMBERS_TABLE_KEY_OUT_NAME = "member";

static void parse_json_string(
        json_value &parent_obj,
        const std::string &parent_name,
        const std::string &member_name,
        std::string &member_value)
{
    const json_value &member_json_value = parent_obj[member_name.c_str()];
    if (&member_json_value != &json_value_none) {
        if (member_json_value.type != json_string) {
            throw dds::core::InvalidArgumentError(
                    "value must be a string: " + parent_name + "."
                    + member_name);
        }
        if (member_json_value.u.string.length == 0) {
            throw dds::core::InvalidArgumentError(
                    "value must be non-empty: " + parent_name + "."
                    + member_name);
        }

        std::string member_value_str(
                member_json_value.u.string.ptr,
                member_json_value.u.string.length);
        member_value = member_value_str;
    } else {
        member_value = "";
    }
}


static void parse_from_json(
        MatchingTable &table,
        const std::string &json_str,
        const std::string &prop_key,
        const std::string &member_in_key,
        const std::string &member_out_name)
{
    RTI_PRCS_FWD_LOG_FN(
            rti::prcs::fwd::property::parse_from_json<MatchingTable>)

    json_value *table_json_value =
            json_parse(json_str.c_str(), json_str.length());
    if (table_json_value == nullptr) {
        json_value_free(table_json_value);
        throw dds::core::InvalidArgumentError(
                "failed to parse JSON: " + json_str);
    }
    if (table_json_value->type != json_array) {
        json_value_free(table_json_value);
        throw dds::core::InvalidArgumentError(
                "property must contain a JSON array: " + prop_key);
    }

    RTI_PRCS_FWD_LOG_1(
            "JSON Matching Table:",
            "%d entries",
            table_json_value->u.array.length)

    for (int i = 0; i < table_json_value->u.array.length; i++) {
        // json_value& entry_json_value = table_json_value[i];
        json_value &entry_json_value = *table_json_value->u.array.values[i];

        if (entry_json_value.type != json_object) {
            throw dds::core::InvalidArgumentError(
                    "table entries must be JSON objects: " + json_str);
        }

        MatchingTableEntry entry;
        std::string member_value;

        parse_json_string(
                entry_json_value,
                prop_key,
                member_in_key,
                member_value);
        if (member_value.length() > 0) {
            entry.in_key() = member_value;
        } else {
            throw dds::core::InvalidArgumentError(
                    "a table entry is missing a required member: property="
                    + prop_key + ", member=" + member_in_key);
        }

        parse_json_string(
                entry_json_value,
                prop_key,
                member_out_name,
                member_value);
        if (member_value.length() > 0) {
            entry.out_name() = member_value;
        } else {
            throw dds::core::InvalidArgumentError(
                    "a table entry is missing a required member: property="
                    + prop_key + ", member=" + member_out_name);
        }

        RTI_PRCS_FWD_LOG_2(
                " - JSON matching entry:",
                "in_key=%s, out_name=%s",
                entry.in_key().c_str(),
                entry.out_name().c_str())

        table.entries().push_back(entry);
    }
    json_value_free(table_json_value);
}

static void parse_matching_table(
        MatchingTable &table,
        const PropertySet &properties,
        const std::string &prop_key,
        const std::string &member_in_key,
        const std::string &member_out_name)
{
    PropertySet::const_iterator it = properties.find(prop_key);
    if (it == properties.end()) {
        throw dds::core::InvalidArgumentError(
                "required property not found: " + prop_key);
    }
    std::string json_str = it->second;
    parse_from_json(table, json_str, prop_key, member_in_key, member_out_name);
}

void property::parse_config(
        const PropertySet &properties,
        ByInputNameForwardingEngineConfiguration &config)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::property::parse_config<
                        ByInputNameForwardingEngineConfiguration>)
    parse_matching_table(
            config.fwd_table(),
            properties,
            property::FORWARDING_TABLE,
            property::FORWARDING_TABLE_KEY_IN_KEY,
            property::FORWARDING_TABLE_KEY_OUT_NAME);
}

void property::parse_config(
        const PropertySet &properties,
        ByInputValueForwardingEngineConfiguration &config)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::property::parse_config<
                        ByInputValueForwardingEngineConfiguration>)

    parse_matching_table(
            config.fwd_table(),
            properties,
            property::FORWARDING_TABLE,
            property::FORWARDING_TABLE_KEY_IN_KEY,
            property::FORWARDING_TABLE_KEY_OUT_NAME);

    InputMembersTable table;
    parse_matching_table(
            table,
            properties,
            property::INPUT_MEMBERS_TABLE,
            property::INPUT_MEMBERS_TABLE_KEY_IN_KEY,
            property::INPUT_MEMBERS_TABLE_KEY_OUT_NAME);
    config.input_members(table);
}
