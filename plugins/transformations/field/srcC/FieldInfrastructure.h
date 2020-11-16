/******************************************************************************/
/* (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#ifndef FieldInfrastructure_h
#define FieldInfrastructure_h

#include "ndds/ndds_c.h"

#include "TransformationInfrastructure.h"
#include "TransformationTypes.h"

#ifdef RTI_TSFM_FIELD_ENABLE_LOG
    #define RTI_TSFM_ENABLE_LOG
#endif /* RTI_TSFM_FIELD_ENABLE_LOG */

#ifdef RTI_TSFM_FIELD_DISABLE_LOG
    #define RTI_TSFM_DISABLE_LOG
#endif /* RTI_TSFM_FIELD_DISABLE_LOG */

#ifdef RTI_TSFM_FIELD_ENABLE_TRACE
    #define RTI_TSFM_ENABLE_TRACE
#endif /* RTI_TSFM_FIELD_ENABLE_TRACE */

/*****************************************************************************
 *                         Configuration Properties
 *****************************************************************************/
#define RTI_TSFM_FIELD_PRIMITIVE_TRANSFORMATION_PROPERTY_PREFIX ""

#define RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_BUFFER_MEMBER \
    RTI_TSFM_FIELD_PRIMITIVE_TRANSFORMATION_PROPERTY_PREFIX "buffer_member"

#define RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_FIELD \
    RTI_TSFM_FIELD_PRIMITIVE_TRANSFORMATION_PROPERTY_PREFIX "field"

#define RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_FIELD_TYPE \
    RTI_TSFM_FIELD_PRIMITIVE_TRANSFORMATION_PROPERTY_PREFIX "field_type"

#define RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_MAX_SERIALIZED_SIZE \
    RTI_TSFM_FIELD_PRIMITIVE_TRANSFORMATION_PROPERTY_PREFIX                  \
            "max_serialized_size"

#define RTI_TSFM_FIELD_PRIMITIVE_PROPERTY_TRANSFORMATION_SERIALIZATION_FORMAT \
    RTI_TSFM_FIELD_PRIMITIVE_TRANSFORMATION_PROPERTY_PREFIX                   \
            "serialization_format"


typedef enum RTI_TSFM_Field_FieldType {
    RTI_TSFM_Field_FieldType_UNKNOWN,
    RTI_TSFM_Field_FieldType_SHORT,
    RTI_TSFM_Field_FieldType_LONG,
    RTI_TSFM_Field_FieldType_USHORT,
    RTI_TSFM_Field_FieldType_ULONG,
    RTI_TSFM_Field_FieldType_FLOAT,
    RTI_TSFM_Field_FieldType_DOUBLE,
    RTI_TSFM_Field_FieldType_BOOLEAN,
    RTI_TSFM_Field_FieldType_CHAR,
    RTI_TSFM_Field_FieldType_OCTET,
    RTI_TSFM_Field_FieldType_STRING,
    RTI_TSFM_Field_FieldType_LONGLONG,
    RTI_TSFM_Field_FieldType_ULONGLONG,
    RTI_TSFM_Field_FieldType_LONGDOUBLE,
    RTI_TSFM_Field_FieldType_WCHAR,
    RTI_TSFM_Field_FieldType_WSTRING
} RTI_TSFM_Field_FieldType;

typedef struct RTI_TSFM_Field_PrimitiveTransformationConfig {
    RTI_TSFM_TransformationConfig parent;
    DDS_Char *buffer_member;
    DDS_Char *field;
    RTI_TSFM_Field_FieldType field_type;
    DDS_UnsignedLong max_serialized_size;
    DDS_Char *serialization_format;
} RTI_TSFM_Field_PrimitiveTransformationConfig;

typedef struct RTI_TSFM_Field_PrimitiveTransformationState {
    DDS_Char *msg_payload;
    DDS_UnsignedLong msg_payload_size;
} RTI_TSFM_Field_PrimitiveTransformationState;

DDS_ReturnCode_t RTI_TSFM_Field_FieldType_from_string(
        const char *str,
        RTI_TSFM_Field_FieldType *tck_out);

#endif /* FieldInfrastructure_h */