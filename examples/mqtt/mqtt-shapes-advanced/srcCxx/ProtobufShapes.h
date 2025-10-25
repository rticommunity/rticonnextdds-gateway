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

#ifndef ProtobufShapes_h
#define ProtobufShapes_h

#ifdef __cplusplus
extern "C" {
#endif

#include "ndds/ndds_c.h"

#ifndef PROTOBUFSHAPES_API
#ifdef _WIN32
#ifdef PROTOBUFSHAPES_EXPORT_DLL
#define PROTOBUFSHAPES_API __declspec(dllexport)
#else
#define PROTOBUFSHAPES_API __declspec(dllimport)
#endif // PROTOBUFSHAPES_EXPORT_DLL
#else
#define PROTOBUFSHAPES_API
#endif // _WIN32
#endif // PROTOBUFSHAPES_API

PROTOBUFSHAPES_API
DDS_ReturnCode_t
ShapeType_protobuf_serialize(
    struct DDS_OctetSeq * const data,
    const char * const color,
    const DDS_Long x,
    const DDS_Long y,
    const DDS_Long shapesize);

PROTOBUFSHAPES_API
DDS_ReturnCode_t
ShapeType_protobuf_deserialize(
    const char *data,
    const size_t data_len,
    DDS_DynamicData * const shape);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ProtobufShapes_h
