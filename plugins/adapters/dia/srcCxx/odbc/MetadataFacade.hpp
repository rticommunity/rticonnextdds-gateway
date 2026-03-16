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

#ifndef METADATAFACADE_HPP
#define METADATAFACADE_HPP

#include <cstdint>
#include <dds/sub/SampleInfo.hpp>
#include <string>
#include <vector>

namespace rti { namespace adapter { namespace dis { namespace odbc {

/**
 * @brief MetadataFacade class. Composed of static methods, it is a collection.
 * of functions necesary to retrieve metadata information from a SampleInfo
 * structure.
 */
class MetadataFacade {

public:
    static std::pair<std::vector<uint8_t>, long> sampleinfo_instance_handle(
            const void *);

    static std::vector<uint8_t> return_instance_handle_as_vector(
            const rti::core::InstanceHandle& instance_handle);

    static void vector_to_instance_handle(
            rti::core::InstanceHandle& instance_handle,
            std::vector<uint8_t> value);

    static std::pair<long long, long> sampleinfo_reception_timestamp_seconds(
            const void *);

    static std::pair<long long, long> sampleinfo_reception_timestamp_nanoseconds(
            const void *);

    /**
     * @brief Extracts the type representation in XCDR format.
     * 
     * @param stream_info Pointer to the stream information structure
     * @return The XCDR serialized type data as a vector of bytes
     */
    static std::vector<uint8_t> extract_type_xcdr(
            const void *stream_info);

    /**
     * @brief Extracts the type representation in XML format.
     * 
     * @param stream_info Pointer to the stream information structure
     * @return The XML type representation as a string
     */
    static std::string extract_type_xml(
            const void *stream_info);

};

} } } } // namespace rti::adapter::dis::odbc

#endif
