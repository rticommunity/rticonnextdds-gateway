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

#include <ndds/ndds_c.h>

#include "DynamicDataFunc.h"

DDS_TypeCode * RTI_COMMON_TypeCode_get_member_type(
    DDS_TypeCode * self,
    const char * member_name)
{
    DDS_UnsignedLong id = 0;
    DDS_TypeCode *member_tc = NULL;
    DDS_ExceptionCode_t ex;
    char *original_name = NULL;

    /* Determine whether it is a nested member or not */
    if (strchr(member_name, '.') == NULL) {
        /* If it is not nested */
        id = DDS_TypeCode_find_member_by_name(self, member_name, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            goto done; /* returns NULL */
        }
        member_tc = DDS_TypeCode_member_type(self, id, &ex);
        if (ex != DDS_NO_EXCEPTION_CODE) {
            goto done; /* returns NULL */
        }
    } else {
        /* Nested TypeCode */
        char *token = NULL;
        const char *delim = ".";
        char *remaining_string = NULL;

        original_name = DDS_String_dup(member_name);
        if (original_name == NULL) {
            goto done; /* returns NULL */
        }

        token = RTIOsapiUtility_strToken(
            original_name,
            delim,
            &remaining_string);

        member_tc = self;

        while (token != NULL) {
            id = DDS_TypeCode_find_member_by_name(member_tc, token, &ex);
            if (ex != DDS_NO_EXCEPTION_CODE) {
                member_tc = NULL;
                goto done;
            }
            member_tc = DDS_TypeCode_member_type(member_tc, id, &ex);
            if (ex != DDS_NO_EXCEPTION_CODE) {
                member_tc = NULL;
                goto done;
            }
            /* Get next token */
            token = RTIOsapiUtility_strToken(
                    NULL,
                    delim,
                    &remaining_string);
        }
        /* At this point member_tc should return the last non-container TC */
    }

done:
    if (original_name != NULL) {
        DDS_String_free(original_name);
    }

    return member_tc;
}
