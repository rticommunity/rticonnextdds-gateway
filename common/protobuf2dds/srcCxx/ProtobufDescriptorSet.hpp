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
#ifndef ProtobufDescriptorSet_hpp
#define ProtobufDescriptorSet_hpp

#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>

namespace rti {
namespace dyndata {
namespace protobuf {

const google::protobuf::Message *
load_message_prototype(
    google::protobuf::DescriptorPool & desc_pool,
    google::protobuf::DynamicMessageFactory & msg_factory,
    const google::protobuf::FileDescriptorSet & desc_set,
    const std::string & msg_name);

google::protobuf::FileDescriptorSet
parse_descriptor_set(const std::string & descriptorFile);


} // namespace protobuf
} // namespace dyndata
} // namespace rti

#endif // ProtobufDescriptorSet_hpp
