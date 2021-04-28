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

#ifndef DynamicDataHelpers_h
#define DynamicDataHelpers_h

#define TYPE_CODE_LENGTH 256

/**
 * @brief Get the TypeCode of an element inside `self`. This name to identify
 * that element may be nested and separated by '.'. For example:
 * "myFirstStruct.mySecondStruct.myElement"
 * @param[in] self the TypeCode which contains the element.
 * @param[in] kind the name of the element to find, it may be a nested element.
 * @return A Typecode of the element found. NULL otherwise.
 */
DDS_TypeCode * RTI_COMMON_TypeCode_get_member_type(
    DDS_TypeCode * self,
    const char * member_name);

/**
 * @brief Add a nul terminator ('\0') at the last element of an octet sequence
 * if it doesn't already end with it. This function increases the length of the
 * sequence by one if there is no room for the nul terminator character.
 * @param[in,out] self the sequence to add the nul character.
 * @return A boolean that says whether the terminator was has been added
 * successfully.
 */
DDS_Boolean RTI_COMMON_DynamicData_octet_seq_assert_nul_terminator(
        struct DDS_OctetSeq *self);

/**
 * @brief Add a nul terminator ('\0') at the last element of an char sequence
 * if it doesn't already end with it. This function increases the length of the
 * sequence by one if there is no room for the nul terminator character.
 * @param[in,out] self the sequence to add the nul character.
 * @return A boolean that says whether the terminator was has been added
 * successfully.
 */
DDS_Boolean RTI_COMMON_DynamicData_char_seq_assert_nul_terminator(
        struct DDS_CharSeq *self);

/**
 * @brief Function that gets the sequence contiguous buffer from a
 * field in a DynamicData object identified by a member name.
 * @param[in] self DynamicData which contains the sequence.
 * @param[out] contiguous_buffer pointer that will be pointing to the contiguous
 * buffer of the sequence.
 * @param[in,out] sequence sequence that will be used to store the value of the
 * DynamicData sequence member. This will perform a ensure_length in the
 * sequence if necessary.
 * @param[in] member the member that identifies the sequence in the DynamicData.
 * @return DDS_RETCODE_OK if everything goes well, a specific retcode if any
 * errors happen.
 */
DDS_ReturnCode_t RTI_COMMON_DynamicData_get_octet_seq_contiguous_buffer(
        DDS_DynamicData *self,
        char **contiguous_buffer,
        struct DDS_OctetSeq *sequence,
        const char *member);

/**
 * @brief Function that gets the sequence contiguous buffer from a
 * field in a DynamicData object identified by a member name.
 * @param[in] self DynamicData which contains the sequence.
 * @param[out] contiguous_buffer pointer that will be pointing to the contiguous
 * buffer of the sequence.
 * @param[in,out] sequence sequence that will be used to store the value of the
 * DynamicData sequence member. This will perform a ensure_length in the
 * sequence if necessary.
 * @param[in] member the member that identifies the sequence in the DynamicData.
 * @return DDS_RETCODE_OK if everything goes well, a specific retcode if any
 * errors happen.
 */
DDS_ReturnCode_t RTI_COMMON_DynamicData_get_char_seq_contiguous_buffer(
        DDS_DynamicData *self,
        char **contiguous_buffer,
        struct DDS_CharSeq *sequence,
        const char *member);

/**
 * @brief Function that sets a DynamicData DDS_OctetSeq element from a string
 * by loaning its buffer.
 * @param[in,out] self DynamicData which contains the sequence.
 * @param[in,out] sequence intermediate sequence that will be used to store on
 * it the value that will be set into the DynamicData sequence member.
 * @param[in] member the member that identifies the DDS_OctetSeq in the
 * DynamicData.
 * @param[in] buffer string which contains the data that will be copied to the
 * DynamicData.
 * @param[in] max_size max allowed size of the sequence
 * @return DDS_RETCODE_OK if everything goes well, a specific retcode if any
 * errors happen.
 */
DDS_ReturnCode_t RTI_COMMON_DynamicData_set_octet_seq_from_string(
        DDS_DynamicData *self,
        struct DDS_OctetSeq *sequence,
        const char *member,
        const char *buffer,
        DDS_UnsignedLong max_size);

/**
 * @brief Function that sets a DynamicData DDS_CharSeq element from a string
 * by loaning its buffer.
 * @param[in,out] self DynamicData which contains the sequence.
 * @param[in,out] sequence intermediate sequence that will be used to store on
 * it the value that will be set into the DynamicData sequence member.
 * @param[in] member the member that identifies the DDS_CharSeq in the
 * DynamicData.
 * @param[in] buffer string which contains the data that will be copied to the
 * DynamicData.
 * @param[in] max_size max allowed size of the sequence
 * @return DDS_RETCODE_OK if everything goes well, a specific retcode if any
 * errors happen.
 */
DDS_ReturnCode_t RTI_COMMON_DynamicData_set_char_seq_from_string(
        DDS_DynamicData *self,
        struct DDS_CharSeq *sequence,
        const char *member,
        const char *buffer,
        DDS_UnsignedLong max_size);


#endif /* DynamicDataHelpers_h */