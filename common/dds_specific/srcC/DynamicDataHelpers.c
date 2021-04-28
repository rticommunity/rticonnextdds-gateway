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

#include "DynamicDataHelpers.h"
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

/**
 * @internal
 * @brief Common function that adds a nul terminator ('\0') at the last element
 * of an octet/char sequence if it doesn't already end with it. This function
 * increases the length of the sequence by one if there is no room for the nul
 * terminator character.
 * This function is internal and shouldn't be used directly, use
 * `RTI_COMMON_DynamicData_<type>_seq_assert_nul_terminator` instead.
 * @param[in,out] self the sequence to add the nul character.
 * @param[in] kind the kind of the elements in the sequence self.
 * @return A boolean that says whether the terminator was has been added
 * successfully.
 * @endinternal
 */
DDS_Boolean RTI_COMMON_DynamicData_seq_assert_nul_terminatorI(
        void *self,
        DDS_TCKind kind)
{
    DDS_Long length = 0;
    struct DDS_OctetSeq *octet_seq = NULL;
    struct DDS_CharSeq *char_seq = NULL;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_seq_assert_nul_terminatorI)

    /* Only DDS_OctetSeq and DDS_CharSeq are currently supported */
    if (kind == DDS_TK_OCTET) {
        octet_seq = (struct DDS_OctetSeq *) self;
    } else if (kind == DDS_TK_CHAR) {
        char_seq = (struct DDS_CharSeq *) self;
    } else {
        RTI_TSFM_ERROR("incompatible datatype to assert terminator");
    }

    /*
     * From now on, we can assume that if we don't have a DDS_OctetSeq, we have
     * a DDS_CharSeq and viceversa
     */
    if (octet_seq != NULL) {
        length = DDS_OctetSeq_get_length(octet_seq);
    } else {
        length = DDS_CharSeq_get_length(char_seq);
    }

    if (length == 0) {
        RTI_TSFM_ERROR(
                "the size of the DDS_OctetSeq is 0, cannot add nul terminator");
        return DDS_BOOLEAN_FALSE;
    }

    /* The sequence is well terminated, no need to add a nul terminator */
    if (octet_seq != NULL) {
        ok = *DDS_OctetSeq_get_reference(octet_seq, length - 1) == '\0';
    } else {
        ok = *DDS_CharSeq_get_reference(char_seq, length - 1) == '\0';
    }
    if (ok) {
        return DDS_BOOLEAN_TRUE;
    }

    /* The nul terminator cannot be added without modifying the length */
    if (octet_seq != NULL) {
        ok = DDS_OctetSeq_ensure_length(octet_seq, length + 1, length + 1);
    } else {
        ok = DDS_CharSeq_ensure_length(char_seq, length + 1, length + 1);
    }
    if (!ok ) {
        RTI_TSFM_ERROR("failed to ensure_length of an octet sequence")
        return DDS_BOOLEAN_FALSE;
    }

    /*
     * length contains the length before increasing it,
     * hence it is pointing ot the new element.
     */
    if (octet_seq != NULL) {
        *DDS_OctetSeq_get_reference(octet_seq, length) = '\0';
    } else {
        *DDS_CharSeq_get_reference(char_seq, length) ='\0';
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean RTI_COMMON_DynamicData_char_seq_assert_nul_terminator(
        struct DDS_CharSeq *self)
{
    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_char_seq_assert_nul_terminator)

    return RTI_COMMON_DynamicData_seq_assert_nul_terminatorI(
            self,
            DDS_TK_CHAR);
}

DDS_Boolean RTI_COMMON_DynamicData_octet_seq_assert_nul_terminator(
        struct DDS_OctetSeq *self)
{
    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_octet_seq_assert_nul_terminator)

    return RTI_COMMON_DynamicData_seq_assert_nul_terminatorI(
            self,
            DDS_TK_OCTET);
}

/**
 * @internal
 * @brief Common function that gets the sequence contiguous buffer from a
 * field in a DynamicData object identified by a member name. This function is
 * internal and shouldn't be used directly, use
 * `RTI_COMMON_DynamicData_get_<type>_seq_contiguous_buffer` instead.
 * @param[in] self DynamicData which contains the sequence.
 * @param[out] contiguous_buffer pointer that will be pointing to the contiguous
 * buffer of the sequence.
 * @param[in,out] sequence sequence that will be used to store the value of the
 * DynamicData sequence member. This will perform a ensure_length in the
 * sequence if necessary.
 * @param[in] member the member that identifies the sequence in the DynamicData.
 * @param[in] kind the kind of the elements in the sequence self.
 * @return DDS_RETCODE_OK if everything goes well, a specific retcode if any
 * errors happen.
 * @endinternal
 */
DDS_ReturnCode_t RTI_COMMON_DynamicData_get_seq_contiguous_bufferI(
        DDS_DynamicData *self,
        char **contiguous_buffer,
        void *sequence,
        const char *member,
        DDS_TCKind kind)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    struct DDS_OctetSeq *octet_seq = NULL;
    struct DDS_CharSeq *char_seq = NULL;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_get_seq_contiguous_bufferI)

    /* Only DDS_OctetSeq and DDS_CharSeq are currently supported */
    if (kind == DDS_TK_OCTET) {
        octet_seq = (struct DDS_OctetSeq *) sequence;
    } else if (kind == DDS_TK_CHAR) {
        char_seq = (struct DDS_CharSeq *) sequence;
    } else {
        RTI_TSFM_ERROR("incompatible datatype to get contiguous buffer");
    }

    /*
     * From now on, we can assume that if we don't have a DDS_OctetSeq, we have
     * a DDS_CharSeq and viceversa
     */
    if (octet_seq != NULL)  {
        retcode = DDS_DynamicData_get_octet_seq(
                self,
                octet_seq,
                member,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    } else {
        retcode = DDS_DynamicData_get_char_seq(
                self,
                char_seq,
                member,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    }
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_ERROR_1("unable to retreive sequences: ", "%s", member)
        goto done;
    }

    /*
     * The buffer might or might not be well terminated with '\0'. We
     * need to ensure that it is.
     */
    if (octet_seq != NULL) {
        ok = RTI_COMMON_DynamicData_octet_seq_assert_nul_terminator(octet_seq);
    } else {
        ok = RTI_COMMON_DynamicData_char_seq_assert_nul_terminator(char_seq);
    }
    if (!ok) {
        RTI_TSFM_ERROR_1("failed to assert nul terminator: ", "%s", member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    /* We have successfully added a nul terminator to the sequence */
    if (octet_seq != NULL) {
        *contiguous_buffer = (char *) DDS_OctetSeq_get_contiguous_buffer(octet_seq);
    } else {
        *contiguous_buffer = (char *) DDS_CharSeq_get_contiguous_buffer(char_seq);
    }
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

DDS_ReturnCode_t RTI_COMMON_DynamicData_get_octet_seq_contiguous_buffer(
        DDS_DynamicData *self,
        char **contiguous_buffer,
        struct DDS_OctetSeq *sequence,
        const char *member)
{
    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_get_octet_seq_contiguous_buffer)

    return RTI_COMMON_DynamicData_get_seq_contiguous_bufferI(
            self,
            contiguous_buffer,
            sequence,
            member,
            DDS_TK_OCTET);
}

DDS_ReturnCode_t RTI_COMMON_DynamicData_get_char_seq_contiguous_buffer(
        DDS_DynamicData *self,
        char **contiguous_buffer,
        struct DDS_CharSeq *sequence,
        const char *member)
{
    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_get_char_seq_contiguous_buffer)

    return RTI_COMMON_DynamicData_get_seq_contiguous_bufferI(
            self,
            contiguous_buffer,
            sequence,
            member,
            DDS_TK_CHAR);
}


/**
 * @internal
 * @brief Common function that sets a DynamicData sequence element from a string
 * by loaning its buffer. Only DDS_CharSeq and DDS_OctetSeq are supported. This
 * function is internal and shouldn't be used directly, use
 * `RTI_COMMON_DynamicData_set_<type>_seq_from_string` instead.
 * @param[in,out] self DynamicData which contains the sequence.
 * @param[in,out] sequence intermediate sequence that will be used to store on
 * it the value that will be set into the DynamicData sequence member.
 * @param[in] member the member that identifies the sequence in the DynamicData.
 * @param[in] buffer string which contains the data that will be copied to the
 * DynamicData.
 * @param[in] max_size max allowed size of the sequence
 * @param[in] kind the kind of the elements in the sequence self.
 * @return DDS_RETCODE_OK if everything goes well, a specific retcode if any
 * errors happen.
 * @endinternal
 */
DDS_ReturnCode_t RTI_COMMON_DynamicData_set_seq_from_stringI(
        DDS_DynamicData *self,
        void *sequence,
        const char *member,
        const char *buffer,
        DDS_UnsignedLong max_size,
        DDS_TCKind kind)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    struct DDS_OctetSeq *octet_seq = NULL;
    struct DDS_CharSeq *char_seq = NULL;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    DDS_Long current_size = 0;

    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_set_seq_from_stringI)

    /*
     * As this will be representing flat data in sequences, we should take into
     * account the nul terminator
     */
    current_size = (DDS_Long) strlen(buffer) + 1;
    if (current_size == 1) {
        RTI_TSFM_ERROR("the buffer is empty");
    }
    if (current_size - 1 == max_size) {
        /* If there is no room for the nul terminator, don't add it */
        current_size--;
    }

    /* Only DDS_OctetSeq and DDS_CharSeq are currently supported */
    if (kind == DDS_TK_OCTET) {
        octet_seq = (struct DDS_OctetSeq *) sequence;
    } else if (kind == DDS_TK_CHAR) {
        char_seq = (struct DDS_CharSeq *) sequence;
    } else {
        RTI_TSFM_ERROR("incompatible datatype to get contiguous buffer");
    }

    /*
     * From now on, we can assume that if we don't have a DDS_OctetSeq, we have
     * a DDS_CharSeq and viceversa
     */
     if (octet_seq != NULL)  {
        ok = DDS_OctetSeq_loan_contiguous(
                octet_seq,
                (DDS_Octet *) buffer,
                current_size,
                max_size);
    } else {
        ok = DDS_CharSeq_loan_contiguous(
                char_seq,
                (DDS_Char *) buffer,
                current_size,
                max_size);
    }
    if (!ok) {
        RTI_TSFM_ERROR_1("unable to loan_contiguous in for member", "%s", member)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    /* Set the sequence in the DynamicData sequence member */
    if (octet_seq != NULL)  {
        retcode = DDS_DynamicData_set_octet_seq(
                self,
                member,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                octet_seq);
    } else {
        retcode = DDS_DynamicData_set_char_seq(
                self,
                member,
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                char_seq);
    }
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_ERROR_1(
            "unable to set_char_seq for member",
            "%s",
            self->config->buffer_member)
        goto done;
    }

    /* Return the loaned buffer in the sequence, this will set max_size = 0 */
    if (octet_seq != NULL)  {
        ok = DDS_OctetSeq_unloan(octet_seq);
    } else {
        ok = DDS_CharSeq_unloan(char_seq);
    }
    if (!ok) {
        RTI_TSFM_ERROR("unable to unloan DDS_Char sequence");
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

done:
    return retcode;
}

DDS_ReturnCode_t RTI_COMMON_DynamicData_set_octet_seq_from_string(
        DDS_DynamicData *self,
        struct DDS_OctetSeq *sequence,
        const char *member,
        const char *buffer,
        DDS_UnsignedLong max_size)
{
    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_set_octet_seq_from_string)

    return RTI_COMMON_DynamicData_set_seq_from_stringI(
            self,
            sequence,
            member,
            buffer,
            max_size,
            DDS_TK_OCTET);
}

DDS_ReturnCode_t RTI_COMMON_DynamicData_set_char_seq_from_string(
        DDS_DynamicData *self,
        struct DDS_CharSeq *sequence,
        const char *member,
        const char *buffer,
        DDS_UnsignedLong max_size)
{
    RTI_TSFM_LOG_FN(RTI_COMMON_DynamicData_set_char_seq_from_string)

    return RTI_COMMON_DynamicData_set_seq_from_stringI(
            self,
            sequence,
            member,
            buffer,
            max_size,
            DDS_TK_CHAR);
}

