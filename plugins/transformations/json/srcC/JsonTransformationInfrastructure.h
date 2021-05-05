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

#ifndef Json_Transformation_Infrastructure_h
#define Json_Transformation_Infrastructure_h

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

#define RTI_TSFM_JSON_BUFFER_SIZE_INCREMENT(buffer_size__) \
        (size_t) 1.5 * (buffer_size__)

#define RTI_TSFM_JSON_UNBOUNDED_INITIAL_SERIALIZED_SIZE_DEFAULT 255
#define RTI_TSFM_JSON_INDENT_DEFAULT 0

typedef struct {
    RTI_TSFM_TransformationConfig parent;
    DDS_Char *buffer_member;
    DDS_UnsignedLong unbounded_initial_serialized_size;
    DDS_UnsignedLong indent;

} RTI_TSFM_JsonTransformationConfig;
typedef struct {
    char *json_buffer;
    DDS_UnsignedLong json_buffer_size;
    DDS_Long json_buffer_bound;
    struct DDS_OctetSeq octet_seq;
    struct DDS_CharSeq char_seq;
} RTI_TSFM_JsonTransformationState;

#define T RTI_TSFM_JsonTransformation
#define TConfig RTI_TSFM_JsonTransformationConfig
#define TState RTI_TSFM_JsonTransformationState
#define T_static
#include "TransformationTemplateDeclare.h"

/*****************************************************************************
 *                         Configuration Properties
 *****************************************************************************/
#define RTI_TSFM_JSON_PROPERTY_PREFIX \
        ""

#define RTI_TSFM_JSON_PROPERTY_BUFFER_MEMBER \
        RTI_TSFM_JSON_PROPERTY_PREFIX "buffer_member"

#define RTI_TSFM_JSON_PROPERTY_SERIALIZED_SIZE_MIN \
        RTI_TSFM_JSON_PROPERTY_PREFIX "serialized_size_min"

#define RTI_TSFM_JSON_PROPERTY_UNBOUNDED_INITIAL_SERIALIZED_SIZE \
        RTI_TSFM_JSON_PROPERTY_PREFIX "unbounded_initial_serialized_size"

#define RTI_TSFM_JSON_PROPERTY_INDENT \
        RTI_TSFM_JSON_PROPERTY_PREFIX "indent"

#endif /* Json_Transformation_Infrastructure_h */