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
#ifndef ProtobufConverterUtils_hpp
#define ProtobufConverterUtils_hpp

#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/reflection.h>

#include "ndds/ndds_c.h"

namespace rti {
namespace dyndata {
namespace protobuf {

struct ProtobufConverterState {
    std::string str_buffer;
    std::map<const DDS_TypeCode*, DDS_DynamicData*> dds_data_cache;

    ~ProtobufConverterState()
    {
        for (auto it : dds_data_cache) {
            DDS_DynamicData_delete(it.second);
        }
        dds_data_cache.clear();
    }

    DDS_DynamicData*
    get_cached_dds_data(const DDS_TypeCode * const ddsTc)
    {
        auto it = dds_data_cache.find(ddsTc);
        if (it != dds_data_cache.end()) {
            return it->second;
        }

        DDS_DynamicData* ddsData = DDS_DynamicData_new(ddsTc, &DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
        if (ddsData != nullptr) {
            dds_data_cache[ddsTc] = ddsData;
        }
        return ddsData;
    }
};


bool
dds_to_protobuf(
    google::protobuf::Message & pbMessage,
    DDS_DynamicData & ddsData,
    ProtobufConverterState * const state = nullptr);

bool
protobuf_to_dds(
    DDS_DynamicData & ddsData,
    const google::protobuf::Message & pbMessage,
    ProtobufConverterState * const state = nullptr);

} // namespace protobuf
} // namespace dyndata
} // namespace rti

#endif // ProtobufConverterUtils_hpp
