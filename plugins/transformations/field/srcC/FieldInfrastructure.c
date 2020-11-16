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

#include "FieldInfrastructure.h"
#include "Transformation.h"
#include "TransformationPlatform.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::field::infrastructure"

DDS_ReturnCode_t RTI_TSFM_Field_FieldType_from_string(
        const char *str,
        RTI_TSFM_Field_FieldType *tck_out)
{
    if (RTI_TSFM_String_compare(str, "short") == 0
        || RTI_TSFM_String_compare(str, "int16") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_SHORT;
        return DDS_RETCODE_OK;
    } else if (
            RTI_TSFM_String_compare(str, "long") == 0
            || RTI_TSFM_String_compare(str, "int32") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_LONG;
        return DDS_RETCODE_OK;
    } else if (
            RTI_TSFM_String_compare(str, "unsigned short") == 0
            || RTI_TSFM_String_compare(str, "uint16") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_USHORT;
        return DDS_RETCODE_OK;
    } else if (
            RTI_TSFM_String_compare(str, "unsigned long") == 0
            || RTI_TSFM_String_compare(str, "uint32") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_ULONG;
        return DDS_RETCODE_OK;
    } else if (RTI_TSFM_String_compare(str, "float") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_FLOAT;
        return DDS_RETCODE_OK;
    } else if (RTI_TSFM_String_compare(str, "double") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_DOUBLE;
        return DDS_RETCODE_OK;
    } else if (
            RTI_TSFM_String_compare(str, "boolean") == 0
            || RTI_TSFM_String_compare(str, "bool") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_BOOLEAN;
        return DDS_RETCODE_OK;
    } else if (RTI_TSFM_String_compare(str, "char") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_CHAR;
        return DDS_RETCODE_OK;
    } else if (RTI_TSFM_String_compare(str, "octet") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_OCTET;
        return DDS_RETCODE_OK;
    } else if (RTI_TSFM_String_compare(str, "string") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_STRING;
        return DDS_RETCODE_OK;
    } else if (
            RTI_TSFM_String_compare(str, "long long") == 0
            || RTI_TSFM_String_compare(str, "int64") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_LONGLONG;
        return DDS_RETCODE_OK;
    } else if (
            RTI_TSFM_String_compare(str, "unsigned long long") == 0
            || RTI_TSFM_String_compare(str, "uint64") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_ULONGLONG;
        return DDS_RETCODE_OK;
    } else if (RTI_TSFM_String_compare(str, "long double") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_LONGDOUBLE;
        return DDS_RETCODE_OK;
    } else if (RTI_TSFM_String_compare(str, "wchar") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_WCHAR;
        return DDS_RETCODE_OK;
    } else if (RTI_TSFM_String_compare(str, "wstring") == 0) {
        *tck_out = RTI_TSFM_Field_FieldType_WSTRING;
        return DDS_RETCODE_OK;
    }

    RTI_TSFM_ERROR_1("unknown field type:", "%s", str)
    return DDS_RETCODE_ERROR;
}
