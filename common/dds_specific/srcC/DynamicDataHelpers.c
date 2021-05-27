/******************************************************************************/
/* (c) 2021 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#include "DynamicDataHelpers.h"
#include "SequenceHelpers.h"
#include "TransformationLog.h"

DDS_TypeCode * RTI_COMMON_TypeCode_get_member_type(
    DDS_TypeCode * self,
    const char * member_name)
{
    DDS_UnsignedLong id = 0;
    DDS_TypeCode *member_tc = NULL;
    DDS_ExceptionCode_t ex = DDS_NO_EXCEPTION_CODE;
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
        char original_name_stack[TYPE_CODE_LENGTH] = "";

        /*
         * strToken modifies the original string, so we dup that string so
         * we can modify it safely
         */
        if (strlen(member_name) > TYPE_CODE_LENGTH - 1) {
            original_name = DDS_String_dup(member_name);
            if (original_name == NULL) {
                goto done; /* returns NULL */
            }
        } else {
            RTIOsapiUtility_strcpy(
                    original_name_stack,
                    TYPE_CODE_LENGTH,
                    member_name);
        }

        /* Get the first token and modify the remaining_string pointer */
        token = RTIOsapiUtility_strToken(
                original_name != NULL ? original_name : original_name_stack,
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

            /* Get next token, remaining_string is the input parameter here */
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

DDS_ReturnCode_t RTI_COMMON_DynamicData_get_octet_seq_contiguous_buffer(
        DDS_DynamicData *self,
        char **contiguous_buffer,
        struct DDS_OctetSeq *seq,
        const char *member)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_get_octet_seq_contiguous_buffer)

    retcode = DDS_DynamicData_get_octet_seq(
            self,
            seq,
            member,
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_ERROR_1("unable to retreive sequences: ", "%s", member)
        goto done;
    }

    /*
     * The buffer might or might not be well terminated with '\0'. We
     * need to ensure that it is.
     */
    ok = DDS_OctetSeq_assert_nul_terminator(seq);
    if (!ok) {
        RTI_TSFM_ERROR_1("failed to assert nul terminator: ", "%s", member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    /* We have successfully added a nul terminator to the sequence */
    *contiguous_buffer = (char *) DDS_OctetSeq_get_contiguous_buffer(seq);
    if (*contiguous_buffer == NULL) {
        RTI_TSFM_ERROR_1(
                "unable to get_contiguous_buffer of element",
                "%s",
                member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

done:
    return retcode;
}

DDS_ReturnCode_t RTI_COMMON_DynamicData_get_char_seq_contiguous_buffer(
        DDS_DynamicData *self,
        char **contiguous_buffer,
        struct DDS_CharSeq *seq,
        const char *member)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_get_char_seq_contiguous_buffer)

    retcode = DDS_DynamicData_get_char_seq(
            self,
            seq,
            member,
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_ERROR_1("unable to retreive sequences: ", "%s", member)
        goto done;
    }

    /*
     * The buffer might or might not be well terminated with '\0'. We
     * need to ensure that it is.
     */
    ok = DDS_CharSeq_assert_nul_terminator(seq);
    if (!ok) {
        RTI_TSFM_ERROR_1("failed to assert nul terminator: ", "%s", member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    /* We have successfully added a nul terminator to the sequence */
    *contiguous_buffer = (char *) DDS_CharSeq_get_contiguous_buffer(seq);
    if (*contiguous_buffer == NULL) {
        RTI_TSFM_ERROR_1(
                "unable to get_contiguous_buffer of element",
                "%s",
                member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

done:
    return retcode;
}

DDS_ReturnCode_t RTI_COMMON_DynamicData_set_octet_seq_from_string(
        DDS_DynamicData *self,
        struct DDS_OctetSeq *seq,
        const char *member,
        const char *buffer,
        DDS_UnsignedLong max_size)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    DDS_Long current_size = 0;

    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_set_octet_seq_from_string)

    /*
     * As this will be representing flat data in sequences, we should take into
     * account the nul terminator
     */
    current_size = (DDS_Long) strlen(buffer) + 1;
    if (current_size == 1) {
        RTI_TSFM_ERROR_1("the buffer is empty for member", "%s", member);
    }
    if (current_size - 1 == max_size) {
        /* If there is no room for the nul terminator, don't add it */
        current_size--;
    }

    ok = DDS_OctetSeq_loan_contiguous(
            seq,
            (DDS_Octet *) buffer,
            current_size,
            max_size);
    if (!ok) {
        RTI_TSFM_ERROR_1("unable to loan_contiguous for member", "%s", member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    /* Set the sequence in the DynamicData sequence member */
    retcode = DDS_DynamicData_set_octet_seq(
            self,
            member,
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            seq);
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_ERROR_1(
            "unable to set_octet_seq for member",
            "%s",
            self->config->buffer_member)
        goto done;
    }

    /* Return the loaned buffer in the sequence, this will set max_size = 0 */
    ok = DDS_OctetSeq_unloan(seq);
    if (!ok) {
        RTI_TSFM_ERROR("unable to unloan DDS_Char sequence");
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

done:
    return retcode;
}

DDS_ReturnCode_t RTI_COMMON_DynamicData_set_char_seq_from_string(
        DDS_DynamicData *self,
        struct DDS_CharSeq *seq,
        const char *member,
        const char *buffer,
        DDS_UnsignedLong max_size)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    DDS_Long current_size = 0;

    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_set_char_seq_from_string)

    /*
     * As this will be representing flat data in sequences, we should take into
     * account the nul terminator
     */
    current_size = (DDS_Long) strlen(buffer) + 1;
    if (current_size == 1) {
        RTI_TSFM_ERROR_1("the buffer is empty for member", "%s", member);
    }
    if (current_size - 1 == max_size) {
        /* If there is no room for the nul terminator, don't add it */
        current_size--;
    }

    ok = DDS_CharSeq_loan_contiguous(
            seq,
            (DDS_Char *) buffer,
            current_size,
            max_size);
    if (!ok) {
        RTI_TSFM_ERROR_1("unable to loan_contiguous in for member", "%s", member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    /* Set the sequence in the DynamicData sequence member */
    retcode = DDS_DynamicData_set_char_seq(
            self,
            member,
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            seq);
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_ERROR_1(
            "unable to set_char_seq for member",
            "%s",
            self->config->buffer_member)
        goto done;
    }

    /* Return the loaned buffer in the sequence, this will set max_size = 0 */
    ok = DDS_CharSeq_unloan(seq);
    if (!ok) {
        RTI_TSFM_ERROR("unable to unloan DDS_Char sequence");
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

done:
    return retcode;
}
