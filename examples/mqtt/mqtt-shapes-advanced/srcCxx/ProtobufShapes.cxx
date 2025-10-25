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

#include "ProtobufShapes.h"
#include "shape_type.pb.h"

DDS_ReturnCode_t
ShapeType_protobuf_serialize(
    struct DDS_OctetSeq * const data,
    const char * const color,
    const DDS_Long x,
    const DDS_Long y,
    const DDS_Long shapesize)
{
    ShapeType shape;

    shape.set_color(color);
    shape.set_x(x);
    shape.set_y(y);
    shape.set_shapesize(shapesize);

    size_t serialized_size = shape.ByteSizeLong();
    size_t data_max_size = DDS_OctetSeq_get_maximum(data);
    if (data_max_size < serialized_size) {
        if (!DDS_OctetSeq_ensure_length(data, serialized_size, serialized_size))
        {
            return DDS_RETCODE_ERROR;
        }
        data_max_size = serialized_size;
    }

    char * data_buf = (char *)DDS_OctetSeq_get_contiguous_buffer(data);
    if (data_buf == nullptr) {
        return DDS_RETCODE_ERROR;
    }

    try {
        shape.SerializeToArray(data_buf, data_max_size);
    } catch (...) {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_OctetSeq_set_length(data, serialized_size))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
ShapeType_protobuf_deserialize(
    const char *data,
    const size_t data_len,
    DDS_DynamicData * const shape)
{
    ShapeType pb_shape;
    if (!pb_shape.ParseFromArray(data, static_cast<int>(data_len))) {
        return DDS_RETCODE_ERROR;
    }
    if (DDS_RETCODE_OK !=
        DDS_DynamicData_set_string(
            shape,
            "color",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            pb_shape.color().c_str()))
    {
        return DDS_RETCODE_ERROR;
    }
    if (DDS_RETCODE_OK !=
        DDS_DynamicData_set_long(
            shape,
            "x",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            pb_shape.x()))
    {
        return DDS_RETCODE_ERROR;
    }
    if (DDS_RETCODE_OK !=
        DDS_DynamicData_set_long(
            shape,
            "y",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            pb_shape.y()))
    {
        return DDS_RETCODE_ERROR;
    }
    if (DDS_RETCODE_OK !=
        DDS_DynamicData_set_long(
            shape,
            "shapesize",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            pb_shape.shapesize()))
    {
        return DDS_RETCODE_ERROR;
    }
    return DDS_RETCODE_OK;
}