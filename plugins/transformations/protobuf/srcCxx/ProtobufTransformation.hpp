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

#ifndef ProtobufTransformation_hpp
#define ProtobufTransformation_hpp

#include <string>

#include "ndds/ndds_c.h"

#include "TransformationLog.h"
#include "TransformationTypes.h"

#include "ProtobufDynamicDataConverter.hpp"

#define RTI_TSFM_PROTO_BUFFER_SIZE_INCREMENT(buffer_size__) \
        ((size_t) (buffer_size__))

typedef struct {
    RTI_TSFM_TransformationConfig parent;
    DDS_Char *buffer_member;
    DDS_Char *descriptor_file;
    DDS_Char *message_type;
} RTI_TSFM_ProtobufTransformationConfig;

typedef struct {
    rti::dyndata::protobuf::ProtobufDynamicDataConverter *msg_converter;
    std::string *message_type;
    std::string *pb_buffer;
    struct DDS_OctetSeq octet_seq;
    struct DDS_CharSeq char_seq;
} RTI_TSFM_ProtobufTransformationState;

#define T RTI_TSFM_ProtobufTransformation
#define TConfig RTI_TSFM_ProtobufTransformationConfig
#define TState RTI_TSFM_ProtobufTransformationState
#define T_static
extern "C" {
#include "TransformationTemplateDeclare.h"
} // extern "C"

/*****************************************************************************
 *                         Configuration Properties
 *****************************************************************************/
#define RTI_TSFM_PROTO_PROPERTY_PREFIX \
        ""

#define RTI_TSFM_PROTO_PROPERTY_BUFFER_MEMBER \
        RTI_TSFM_PROTO_PROPERTY_PREFIX "buffer_member"

#define RTI_TSFM_PROTO_PROPERTY_DESCRIPTOR_FILE \
        RTI_TSFM_PROTO_PROPERTY_PREFIX "descriptor_file"

#define RTI_TSFM_PROTO_PROPERTY_MESSAGE_TYPE \
        RTI_TSFM_PROTO_PROPERTY_PREFIX "message_type"

#define RTI_TSFM_PROTO_UNBOUNDED_MEMBER_SERIALIZED_SIZE_INITIAL_DEFAULT 256

#endif /* ProtobufTransformation_hpp */
