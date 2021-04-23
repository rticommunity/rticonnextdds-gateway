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

#ifndef Flat_Type_Infrastructure_h
#define Flat_Type_Infrastructure_h

#ifdef RTI_TSFM_JSON_ENABLE_LOG
    #define RTI_TSFM_ENABLE_LOG
#endif /* RTI_TSFM_JSON_ENABLE_LOG */

#ifdef RTI_TSFM_JSON_DISABLE_LOG
    #define RTI_TSFM_DISABLE_LOG
#endif /* RTI_TSFM_JSON_DISABLE_LOG */

#ifdef RTI_TSFM_JSON_ENABLE_TRACE
    #define RTI_TSFM_ENABLE_TRACE
#endif /* RTI_TSFM_JSON_ENABLE_TRACE */

#include "ndds/ndds_c.h"

#include "TransformationTypes.h"

typedef struct RTI_TSFM_Json_FlatTypeTransformationConfig {
    RTI_TSFM_TransformationConfig parent;
    DDS_Char *buffer_member;
    DDS_Long serialized_size_max;
    DDS_UnsignedLong serialized_size_min;
    DDS_UnsignedLong serialized_size_incr;
    DDS_UnsignedLong indent;

} RTI_TSFM_Json_FlatTypeTransformationConfig;

struct RTI_TSFM_Json_FlatTypeTransformationImpl;

typedef struct RTI_TSFM_Json_FlatTypeTransformation_MemberMappingImpl
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping;
struct RTI_TSFM_Json_FlatTypeTransformation_MemberMappingImpl {
    DDS_UnsignedLong id;
    DDS_TCKind kind;
    DDS_UnsignedLong max_len;
    DDS_Boolean optional;
    const char *name;
};

DDS_SEQUENCE(
        RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq,
        RTI_TSFM_Json_FlatTypeTransformation_MemberMapping);


typedef struct RTI_TSFM_Json_FlatTypeTransformationStateImpl {
    char *json_buffer;
    DDS_UnsignedLong json_buffer_size;
    struct DDS_OctetSeq octet_seq;
    struct RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq
            output_mappings;
    struct RTI_TSFM_Json_FlatTypeTransformation_MemberMappingSeq input_mappings;
} RTI_TSFM_Json_FlatTypeTransformationState;

#define T RTI_TSFM_Json_FlatTypeTransformation
#define TConfig RTI_TSFM_Json_FlatTypeTransformationConfig
#define TState RTI_TSFM_Json_FlatTypeTransformationState
#define T_static
#include "TransformationTemplateDeclare.h"

/*****************************************************************************
 *                         Configuration Properties
 *****************************************************************************/
#define RTI_TSFM_JSON_FLATTYPE_TRANSFORMATION_PROPERTY_PREFIX \
        ""

#define RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_BUFFER_MEMBER \
        RTI_TSFM_JSON_FLATTYPE_TRANSFORMATION_PROPERTY_PREFIX "buffer_member"

#define RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_SERIALIZED_SIZE_MIN \
        RTI_TSFM_JSON_FLATTYPE_TRANSFORMATION_PROPERTY_PREFIX "serialized_size_min"

#define RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_SERIALIZED_SIZE_MAX \
        RTI_TSFM_JSON_FLATTYPE_TRANSFORMATION_PROPERTY_PREFIX "serialized_size_max"

#define RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_SERIALIZED_SIZE_INCR \
        RTI_TSFM_JSON_FLATTYPE_TRANSFORMATION_PROPERTY_PREFIX "serialized_size_incr"

#define RTI_TSFM_JSON_FLATTYPE_PROPERTY_TRANSFORMATION_INDENT \
        RTI_TSFM_JSON_FLATTYPE_TRANSFORMATION_PROPERTY_PREFIX "indent"

#endif /* Flat_Type_Infrastructure_t */