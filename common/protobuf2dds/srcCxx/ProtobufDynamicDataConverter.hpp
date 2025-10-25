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
#ifndef ProtobufDynamicDataConverter_hpp
#define ProtobufDynamicDataConverter_hpp

#include <fstream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>

#include "DynamicDataConverter.hpp"
#include "ProtobufConverterUtils.hpp"

namespace rti {
namespace dyndata {
namespace protobuf {

class ProtobufDynamicDataConverter : public DynamicDataConverter {

public:
    ProtobufDynamicDataConverter(
        const google::protobuf::FileDescriptorSet & pbDescriptorSet,
        const std::string & pbMessageName);

    virtual ~ProtobufDynamicDataConverter();

    bool
    deserialize(
        DDS_DynamicData & data,
        const char * const serialized_data,
        const size_t serialized_data_len);

    bool
    serialize(
        DDS_DynamicData & data,
        std::string & serialized_data);

protected:
    ProtobufConverterState conv_state_;
    google::protobuf::DescriptorPool pb_desc_pool_;
    google::protobuf::DynamicMessageFactory pb_msg_factory_;
    const google::protobuf::Message *pb_msg_proto_;
    google::protobuf::Message *pb_msg_;
};


} // namespace protobuf
} // namespace dyndata
} // namespace rti

#endif // ProtobufDynamicDataConverter_hpp
