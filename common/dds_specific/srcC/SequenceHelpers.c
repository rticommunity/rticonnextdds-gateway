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

#include "SequenceHelpers.h"
#include "TransformationLog.h"

DDS_Boolean DDS_CharSeq_assert_nul_terminator(struct DDS_CharSeq *self)
{
    DDS_Long length = 0;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(DDS_CharSeq_assert_nul_terminator)

    length = DDS_CharSeq_get_length(self);

    if (length == 0) {
        RTI_TSFM_ERROR(
                "the size of the DDS_CharSeq is 0, cannot add nul terminator");
        return DDS_BOOLEAN_FALSE;
    }

    /* The sequence is well terminated, no need to add a nul terminator */
    ok = *DDS_CharSeq_get_reference(self, length - 1) == '\0';
    if (ok) {
        return DDS_BOOLEAN_TRUE;
    }

    /*
     * The nul terminator cannot be added without modifying the length. The
     * set_length function will modify the maximum if needed, but we don't want
     * to modify the maximum if it is already enough.
     */
    ok = DDS_CharSeq_set_length(self, length + 1);
    if (!ok) {
        RTI_TSFM_ERROR("failed to set_length of a DDS_CharSeq")
        return DDS_BOOLEAN_FALSE;
    }

    /*
     * length contains the length before increasing it,
     * hence it is pointing to the new element.
     */
    *DDS_CharSeq_get_reference(self, length) ='\0';

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean DDS_OctetSeq_assert_nul_terminator(struct DDS_OctetSeq *self)
{
    DDS_Long length = 0;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(DDS_OctetSeq_assert_nul_terminator)

    length = DDS_OctetSeq_get_length(self);
    if (length == 0) {
        RTI_TSFM_ERROR(
                "the size of the DDS_OctetSeq is 0, cannot add nul terminator");
        return DDS_BOOLEAN_FALSE;
    }

    /* The sequence is well terminated, no need to add a nul terminator */
    ok = *DDS_OctetSeq_get_reference(self, length - 1) == '\0';
    if (ok) {
        return DDS_BOOLEAN_TRUE;
    }

    /*
     * The nul terminator cannot be added without modifying the length. The
     * set_length function will modify the maximum if needed, but we don't want
     * to modify the maximum if it is already enough.
     */
    ok = DDS_OctetSeq_set_length(self, length + 1);
    if (!ok) {
        RTI_TSFM_ERROR("failed to set_length of a DDS_OctetSeq")
        return DDS_BOOLEAN_FALSE;
    }

    /*
     * length contains the length before increasing it,
     * hence it is pointing to the new element.
     */
    *DDS_OctetSeq_get_reference(self, length) = '\0';

    return DDS_BOOLEAN_TRUE;
}
