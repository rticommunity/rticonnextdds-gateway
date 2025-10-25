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
#include "ProtobufDynamicDataConverter.hpp"
#include "ProtobufDescriptorSet.hpp"
#include "ProtobufConverterUtils.hpp"
#include "TransformationLog.h"

#define RTI_TSFM_LOG_ARGS "pbconverter"

namespace rti {
namespace dyndata {
namespace protobuf {

ProtobufDynamicDataConverter::ProtobufDynamicDataConverter(
    const google::protobuf::FileDescriptorSet & pbDescriptorSet,
    const std::string & pbMessageName)
{
    pb_msg_proto_ = load_message_prototype(
        pb_desc_pool_,
        pb_msg_factory_,
        pbDescriptorSet,
        pbMessageName);
    if (pb_msg_proto_ == nullptr) {
        RTI_TSFM_ERROR_1(
            "failed to load protobuf message",
            "%s",
            pbMessageName.c_str())
        throw std::runtime_error("failed to load protobuf message");
    }

    pb_msg_ = pb_msg_proto_->New();
    if (pb_msg_ == nullptr) {
        RTI_TSFM_ERROR_1(
            "failed to create new protobuf message",
            "%s",
            pb_msg_proto_->GetDescriptor()->full_name().c_str())
        throw std::runtime_error("failed to create new protobuf message");
    }
}

ProtobufDynamicDataConverter::~ProtobufDynamicDataConverter()
{
    if (pb_msg_ != nullptr) {
        delete pb_msg_;
        pb_msg_ = nullptr;
    }
    // pb_msg_proto_ is owned by pb_msg_factory_
}

bool
ProtobufDynamicDataConverter::deserialize(
    DDS_DynamicData & data,
    const char * const serialized_data,
    const size_t serialized_data_len)
{
    pb_msg_->Clear();
    if (!pb_msg_->ParseFromArray(serialized_data, static_cast<int>(serialized_data_len))) {
        RTI_TSFM_ERROR_1(
            "failed to parse protobuf message from array",
            "%s",
            pb_msg_proto_->GetDescriptor()->full_name().c_str())
        return false;
    }

    if (!protobuf_to_dds(data, *pb_msg_, &conv_state_)) {
        RTI_TSFM_ERROR_1(
            "failed to convert from protobuf message",
            "%s",
            pb_msg_proto_->GetDescriptor()->full_name().c_str())
        return false;
    }

    return true;
}

bool
ProtobufDynamicDataConverter::serialize(
    DDS_DynamicData & data,
    std::string & serialized_data)
{
    pb_msg_->Clear();

    if (!dds_to_protobuf(*pb_msg_, data, &conv_state_)) {
        RTI_TSFM_ERROR_1(
            "failed to convert to protobuf message",
            "%s",
            pb_msg_proto_->GetDescriptor()->full_name().c_str())
        return false;
    }

    if (!pb_msg_->SerializeToString(&serialized_data)) {
        RTI_TSFM_ERROR_1(
            "failed to serialize protobuf message to string",
            "%s",
            pb_msg_proto_->GetDescriptor()->full_name().c_str())
        return false;
    }

    return true;
}

} // namespace protobuf
} // namespace dyndata
} // namespace rti