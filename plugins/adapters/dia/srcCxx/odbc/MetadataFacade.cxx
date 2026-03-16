/******************************************************************************/
/* (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#include "MetadataFacade.hpp"
#include "Utils.hpp"
#include <rti/routing/StreamInfo.hpp>
#include <ndds/ndds_c.h>

namespace rti { namespace adapter { namespace dis { namespace odbc {

using namespace dds::core::xtypes;

std::pair<std::vector<uint8_t>, long> MetadataFacade::sampleinfo_instance_handle(
        const void *info)
{
    std::vector<uint8_t> value(return_instance_handle_as_vector(
            ((dds::sub::SampleInfo *)info)->instance_handle()));
    auto value_size = sizeof(uint8_t) * (value.capacity());

    return std::pair<std::vector<uint8_t>, long>(value, value_size);
}

std::vector<uint8_t> MetadataFacade::return_instance_handle_as_vector(
        const rti::core::InstanceHandle& instance_handle)
{
    std::vector<uint8_t> generic_octet_array;
    generic_octet_array.assign(
            (uint8_t *) instance_handle.native().keyHash.value,
            (uint8_t *) (instance_handle.native().keyHash.value
                    + MIG_RTPS_KEY_HASH_MAX_LENGTH));
    return generic_octet_array;
}

void MetadataFacade::vector_to_instance_handle(
        rti::core::InstanceHandle& instance_handle,
        std::vector<uint8_t> value)
{
    std::copy(value.begin(), value.end(), instance_handle.native().keyHash.value);
}

std::pair<long long, long> MetadataFacade::
sampleinfo_reception_timestamp_seconds(const void *info)
{
    auto value = ((dds::sub::SampleInfo *)info)->extensions().native().
            reception_timestamp.sec;
    auto value_size = sizeof(DDS_Long);
    return std::pair<long, long>(value, value_size);
}

std::pair<long long, long> MetadataFacade::
sampleinfo_reception_timestamp_nanoseconds(const void *info)
{
    auto value = ((dds::sub::SampleInfo *)info)->extensions().native().
            reception_timestamp.nanosec;
    auto value_size = sizeof(DDS_UnsignedLong);
    return std::pair<long, long>(value, value_size);
}

std::vector<uint8_t> MetadataFacade::extract_type_xcdr(
        const void* stream_info)
{
    std::vector<uint8_t> xcdr_data;

    const auto* info = static_cast<const rti::routing::StreamInfo*>(stream_info);
    const dds::core::xtypes::DynamicType dynamic_type = 
            info->type_info().dynamic_type();

    // Get a pointer to the native TypeCode
    const DDS_TypeCode * native_tc = &dynamic_type.native();
    if (native_tc != nullptr) {
        DDS_TypeObject * type_object =
                DDS_TypeObject_create_from_typecode(native_tc);
        if (type_object == NULL) {
            throw std::runtime_error("Failed to create type-object");
        }

        if (type_object != NULL) {
            /* Get the expected length of the serialized type-object, then create a
             * buffer big enough to hold it and then serialize the type-object */
            unsigned int data_size =
                    DDS_TypeObject_get_serialized_size(type_object);
            xcdr_data.resize(data_size);
            if (DDS_TypeObject_serialize(
                    type_object,
                    (char *) &(xcdr_data[0]),
                    &data_size) != DDS_RETCODE_OK) {
                DDS_TypeObject_delete(type_object);
                throw std::runtime_error("Failed to serialize type-object");
            }

            DDS_TypeObject_delete(type_object);
        }
    }

    return xcdr_data;
}

std::string MetadataFacade::extract_type_xml(
        const void *stream_info)
{
    std::string xml_data;

    const auto* info = static_cast<const rti::routing::StreamInfo*>(stream_info);
    const dds::core::xtypes::DynamicType& dynamic_type = 
            info->type_info().dynamic_type();

    const DDS_TypeCode * native_tc = &dynamic_type.native();
    if (native_tc != nullptr) {
        DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
        char* idl_string = nullptr;
        DDS_UnsignedLong str_size = 0;
        DDS_TypeCode_to_string(native_tc, idl_string, &str_size, &ex);

        if (ex == DDS_NO_EXCEPTION_CODE && idl_string != nullptr) {
            xml_data = std::string(idl_string);
            DDS_String_free(idl_string);
        }
    }

    return xml_data;
}

} } } } // namespace rti::adapter::dis::odbc
