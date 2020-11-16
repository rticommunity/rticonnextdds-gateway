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

using namespace dds::core::xtypes;
using namespace rti::routing;
using namespace rti::routing::processor;
using namespace rti::routing::adapter;
using namespace rti::prcs::fwd;

#define RTI_PRCS_FWD_LOG_ARGS "rti::prcs::fwd::ByInputValueForwardingEngine"

const char *InputMemberValue::FORMAT_BOOLEAN = "%d";
const char *InputMemberValue::FORMAT_UINT8 = "%u";
const char *InputMemberValue::FORMAT_UINT16 = "%u";
const char *InputMemberValue::FORMAT_UINT32 = "%u";
const char *InputMemberValue::FORMAT_UINT64 = "%lu";
const char *InputMemberValue::FORMAT_CHAR8 = "%c";
const char *InputMemberValue::FORMAT_INT16 = "%d";
const char *InputMemberValue::FORMAT_INT32 = "%d";
const char *InputMemberValue::FORMAT_INT64 = "%ld";
const char *InputMemberValue::FORMAT_FLOAT32 = "%f";
const char *InputMemberValue::FORMAT_FLOAT64 = "%f";
const char *InputMemberValue::FORMAT_STRING = "%s";

template <class T>
std::string print_input_field(const std::string &format, const T &val)
{
    RTI_PRCS_FWD_LOG_FN(print_input_field)

    size_t print_size = snprintf(nullptr, 0, format.c_str(), val);

    if (print_size <= 0) {
        throw dds::core::InvalidArgumentError("cannot format value");
    }

    std::unique_ptr<char[]> print_buffer(new char[print_size + 1]);

    snprintf(print_buffer.get(), print_size + 1, format.c_str(), val);

    return std::string(print_buffer.get(), print_buffer.get() + print_size);
}

template <class T>
std::string format_primitive_field(
        InputMemberValue *mapping,
        const DynamicData &data)
{
    RTI_PRCS_FWD_LOG_FN(format_primitive_field)
    return print_input_field<T>(
            mapping->string_format,
            data.value<T>(mapping->name));
}

static std::string default_input_member_format(const TypeKind &tk)
{
    RTI_PRCS_FWD_LOG_FN(default_input_member_format)

    std::string input_fmt;

    switch (tk.underlying()) {
    case TypeKind::BOOLEAN_TYPE:
        input_fmt = InputMemberValue::FORMAT_BOOLEAN;
        break;
    case TypeKind::UINT_8_TYPE:
        input_fmt = InputMemberValue::FORMAT_UINT8;
        break;
    case TypeKind::UINT_16_TYPE:
        input_fmt = InputMemberValue::FORMAT_UINT16;
        break;
    case TypeKind::UINT_32_TYPE:
        input_fmt = InputMemberValue::FORMAT_UINT32;
        break;
    case TypeKind::UINT_64_TYPE:
        input_fmt = InputMemberValue::FORMAT_UINT64;
        break;
    case TypeKind::CHAR_8_TYPE:
        input_fmt = InputMemberValue::FORMAT_CHAR8;
        break;
    case TypeKind::INT_16_TYPE:
        input_fmt = InputMemberValue::FORMAT_INT16;
        break;
    case TypeKind::INT_32_TYPE:
        input_fmt = InputMemberValue::FORMAT_INT32;
        break;
    case TypeKind::INT_64_TYPE:
        input_fmt = InputMemberValue::FORMAT_INT64;
        break;
    case TypeKind::FLOAT_32_TYPE:
        input_fmt = InputMemberValue::FORMAT_FLOAT32;
        break;
    case TypeKind::FLOAT_64_TYPE:
        input_fmt = InputMemberValue::FORMAT_FLOAT64;
        break;
    case TypeKind::STRING_TYPE:
        input_fmt = InputMemberValue::FORMAT_STRING;
        break;
    default:
        throw dds::core::InvalidArgumentError("unsupported member type");
    }

    return input_fmt;
}

InputMemberValue::InputMemberValue()
{
}


InputMemberValue::InputMemberValue(
        const char *name,
        const dds::core::xtypes::DynamicData &data)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::InputMemberValue::InputMemberValue)

    this->name = name;
    this->info.set(data.member_info(this->name));
    this->string_format =
            default_input_member_format(this->info.get().member_kind());
}

void InputMemberValue::to_string(const DynamicData &data, std::string &str_out)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::InputMemberValue::to_string)

    if (!data.member_exists(this->name)) {
        throw dds::core::InvalidArgumentError(
                "input member not found in sample: " + this->name);
    }

    if (!this->info.is_set()) {
        throw dds::core::InvalidArgumentError(
                "not type information for input member: " + this->name);
    }

    switch (this->info.get().member_kind().underlying()) {
    case TypeKind::BOOLEAN_TYPE:
        str_out = format_primitive_field<bool>(this, data);
        break;
    case TypeKind::UINT_8_TYPE:
        str_out = format_primitive_field<uint8_t>(this, data);
        break;
    case TypeKind::UINT_16_TYPE:
        str_out = format_primitive_field<uint16_t>(this, data);
        break;
    case TypeKind::UINT_32_TYPE:
        str_out = format_primitive_field<uint32_t>(this, data);
        break;
    case TypeKind::UINT_64_TYPE:
        str_out = format_primitive_field<uint64_t>(this, data);
        break;
    case TypeKind::CHAR_8_TYPE:
        str_out = format_primitive_field<char>(this, data);
        break;
    case TypeKind::INT_16_TYPE:
        str_out = format_primitive_field<int16_t>(this, data);
        break;
    case TypeKind::INT_32_TYPE:
        str_out = format_primitive_field<int32_t>(this, data);
        break;
    case TypeKind::INT_64_TYPE:
        str_out = format_primitive_field<int64_t>(this, data);
        break;
    case TypeKind::FLOAT_32_TYPE:
        str_out = format_primitive_field<float>(this, data);
        break;
    case TypeKind::FLOAT_64_TYPE:
        str_out = format_primitive_field<double>(this, data);
        break;
    case TypeKind::STRING_TYPE:
        str_out = data.value<std::string>(this->name);
        break;
    default:
        /* Should never get here */
        throw dds::core::InvalidArgumentError("unexpected input member type");
    }
}

ByInputValueForwardingEngine::ByInputValueForwardingEngine(
        ByInputValueForwardingEngineConfiguration &config)
        : ForwardingEngine(config)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ByInputValueForwardingEngine::
                                ByInputValueForwardingEngine)

    input_members =
            InternalMatchingTable::from_matching_table(config.input_members());
}

void ByInputValueForwardingEngine::get_forwarding_key(
        TypedInput<DynamicData> &input,
        const DynamicData &data,
        std::string &fwd_key_out)
{
    RTI_PRCS_FWD_LOG_FN(
            rti::prcs::fwd::ByInputValueForwardingEngine::get_forwarding_key)

    InputMemberValue &mem_value = cache_input_member(input, data);
    mem_value.to_string(data, fwd_key_out);

    RTI_PRCS_FWD_TRACE_3(
            "forwarding KEY:",
            "input=%s, data=%p, key=%s",
            input.name().c_str(),
            data,
            fwd_key_out.c_str())
}

std::map<std::string, InputMemberValue> &
        ByInputValueForwardingEngine::get_input_map(
                TypedInput<dds::core::xtypes::DynamicData> &input)
{
    const std::string &in_name = input.name();
    std::map<std::string, std::map<std::string, InputMemberValue>>::
            const_iterator input_it = input_members_cache.find(in_name);

    if (input_it == input_members_cache.end()) {
        /* no entry for this input, create one */
        std::map<std::string, InputMemberValue> in_map;
        input_members_cache[in_name] = in_map;
    }

    return input_members_cache[in_name];
}

InputMemberValue &ByInputValueForwardingEngine::cache_input_member(
        TypedInput<dds::core::xtypes::DynamicData> &input,
        const dds::core::xtypes::DynamicData &data)
{
    std::map<std::string, InputMemberValue> &in_map = get_input_map(input);

    InternalMatchingTableEntry &in_mem_entry =
            input_members.find(input.name().c_str());

    std::map<std::string, InputMemberValue>::iterator in_mem_it =
            in_map.find(in_mem_entry.out_name);

    if (in_mem_it != in_map.end()) {
        /* input already cached */
        return in_mem_it->second;
    }

    InputMemberValue mem_entry(in_mem_entry.out_name.c_str(), data);
    in_map[in_mem_entry.out_name] = mem_entry;

    return in_map[in_mem_entry.out_name];
}

Processor *ByInputValueForwardingEnginePlugin::create_processor(
        Route &route,
        const PropertySet &properties)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ByInputValueForwardingEnginePlugin::
                                create_processor)

    ByInputValueForwardingEngineConfiguration config;

    property::parse_config(properties, config);

    return new ByInputValueForwardingEngine(config);
}

void ByInputValueForwardingEnginePlugin::delete_processor(
        rti::routing::processor::Route &route,
        rti::routing::processor::Processor *processor)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ByInputValueForwardingEnginePlugin::
                                delete_processor)
    delete processor;
}

ByInputValueForwardingEnginePlugin::ByInputValueForwardingEnginePlugin(
        const rti::routing::PropertySet &)
{
    RTI_PRCS_FWD_LOG_FN(rti::prcs::fwd::ByInputValueForwardingEnginePlugin::
                                ByInputValueForwardingEnginePlugin)
}

struct RTI_RoutingServiceProcessorPlugin *
        RTI_PRCS_FWD_ByInputValueForwardingEnginePlugin_create(
                const struct RTI_RoutingServiceProperties *native_properties,
                RTI_RoutingServiceEnvironment *environment)
{
    RTI_PRCS_FWD_LOG_FN(RTI_PRCS_FWD_ByInputValueForwardingEnginePlugin_create)

    PropertySet properties;
    rti::routing::PropertyAdapter::add_properties_from_native(
            properties,
            native_properties);
    try {
        return rti::routing::processor::detail::ProcessorPluginForwarder::
                create_plugin(
                        new ByInputValueForwardingEnginePlugin(properties));
    } catch (const std::exception &ex) {
        RTI_RoutingServiceEnvironment_set_error(environment, "%s", ex.what());
    } catch (...) {
    }

    return NULL;
}