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
 * @brief Get the TypeCode of the element, including nested elements.
 * @param self the main TypeCode object.
 * @param member_name the element name that identifies the member.
 * @return the TypeCode of the member. NULL in case of errors.
 */
DDS_TypeCode * RTI_COMMON_TypeCode_get_member_type(
    DDS_TypeCode * self,
    const char * member_name);

#endif /* DynamicDataHelpers_h */