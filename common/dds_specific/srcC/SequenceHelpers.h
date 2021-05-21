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

#ifndef SequenceHelpers_h
#define SequenceHelpers_h

#include <ndds/ndds_c.h>

/**
 * @brief Make sure that a octet sequence ends with a "nul" terminator.
 *
 * This function allows the value stored in the sequence to be interpreted as
 * a C string.
 *
 * Given a sequence `seq` of length `L` and maximum `M`, this function will
 * will make sure that `seq[L - 1] == '\0'`. The terminator will be added to the
 * sequence as an additional element, if the sequence doesn't already end
 * with it (i.e. the sequence will be resized to `L + 1`). The sequence will
 * also be resized to `M + 1` if it doesn't have space for additional elements
 * (i.e. `M < L + 1`).
 *
 * Note that this function will not validate the contents of the sequence
 * beside checking its last element. For example, it will not make sure that
 * the sequence doesn't already contain a "nul" terminator at an earlier
 * position.
 *
 * @param[in,out] self the sequence in which to assert the "nul" terminator.
 * @return True if the sequence is now "null" terminated, False if an error
 * occurred while asserting the terminator.
 */
DDS_Boolean DDS_OctetSeq_assert_nul_terminator(struct DDS_OctetSeq *self);

/**
 * @brief Make sure that a character sequence ends with a "nul" terminator.
 *
 * This function allows the value stored in the sequence to be interpreted as
 * a C string.
 *
 * Given a sequence `seq` of length `L` and maximum `M`, this function will
 * will make sure that `seq[L - 1] == '\0'`. The terminator will be added to the
 * sequence as an additional element, if the sequence doesn't already end
 * with it (i.e. the sequence will be resized to `L + 1`). The sequence will
 * also be resized to `M + 1` if it doesn't have space for additional elements
 * (i.e. `M < L + 1`).
 *
 * Note that this function will not validate the contents of the sequence
 * beside checking its last element. For example, it will not make sure that
 * the sequence doesn't already contain a "nul" terminator at an earlier
 * position.
 * @param[in,out] self the sequence in which to assert the "nul" terminator.
 * @return True if the sequence is now "null" terminated, False if an error
 * occurred while asserting the terminator.
 */
DDS_Boolean DDS_CharSeq_assert_nul_terminator(struct DDS_CharSeq *self);

#endif /* SequenceHelpers_h */
