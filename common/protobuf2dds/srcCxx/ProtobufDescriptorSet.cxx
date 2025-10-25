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
#include <fstream>

#include "ProtobufDescriptorSet.hpp"

#include "TransformationLog.h"

#define RTI_TSFM_LOG_ARGS "pbdescriptor"

namespace rti {
namespace dyndata {
namespace protobuf {

const google::protobuf::Message *
load_message_prototype(
        google::protobuf::DescriptorPool & desc_pool,
        google::protobuf::DynamicMessageFactory & msg_factory,
        const google::protobuf::FileDescriptorSet & desc_set,
        const std::string & msg_name)
{
    for (int i = 0; i < desc_set.file_size(); ++i) {
        const google::protobuf::FileDescriptorProto & file_desc_proto =
            desc_set.file(i);
        const google::protobuf::FileDescriptor * file_desc =
            desc_pool.BuildFile(file_desc_proto);
        if (file_desc == nullptr) {
            RTI_TSFM_ERROR_1(
                "failed to build file descriptor for",
                "%s",
                file_desc_proto.name().c_str())
            return nullptr;
        }
        for (int j = 0; j < file_desc->message_type_count(); ++j) {
            const google::protobuf::Descriptor * msg_desc =
                file_desc->message_type(j);
            if (msg_desc == nullptr) {
                RTI_TSFM_ERROR_2(
                    "failed to get message descriptor for",
                    "%s in %s",
                    msg_desc->name().c_str(),
                    file_desc->name().c_str())
                return nullptr;
            }
            if (msg_name == msg_desc->full_name()) {
                const google::protobuf::Message *msg = msg_factory.GetPrototype(
                    const_cast<google::protobuf::Descriptor*>(msg_desc));
                if (msg == nullptr) {
                    RTI_TSFM_ERROR_1(
                        "failed to get message prototype for",
                        "%s",
                        msg_desc->full_name().c_str())
                    return nullptr;
                }
                return msg;
            }
        }
    }
    RTI_TSFM_ERROR_1(
        "message not found in descriptor set",
        "%s",
        msg_name.c_str())
    return nullptr;
}

google::protobuf::FileDescriptorSet
parse_descriptor_set(const std::string & descriptorFile)
{
    google::protobuf::FileDescriptorSet descriptorSet;
    std::ifstream input(descriptorFile, std::ios::binary);
    descriptorSet.ParseFromIstream(&input);    
    return std::move(descriptorSet);
}

} // namespace protobuf
} // namespace dyndata
} // namespace rti