/******************************************************************************/
/* (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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
#ifndef ProtobufMessageUtils_hpp
#define ProtobufMessageUtils_hpp

#include "message.pb.h"

namespace test {

void
generate_bytes_data(std::string & bytes, const int32_t count);

void
generate_anothermessage(
    ::test::AnotherMessage& another_message,
    const int32_t count);

void
verify_anothermessage(
    const ::test::AnotherMessage& another_message,
    const int32_t count);

void
generate_allrequiredmessage(
    ::test::AllRequiredMessage& all_required_message,
    const int32_t count);

void
verify_allrequiredmessage(
    const ::test::AllRequiredMessage& all_required_message,
    const int32_t count);

void
generate_submessage(
    ::test::Message_SubMessage& submessage,
    const int32_t count);

void
verify_submessage(
    const ::test::Message_SubMessage& submessage,
    const int32_t count);

void
generate_message(
    ::test::Message& message,
    const int32_t tester_id,
    const int32_t count);

void
verify_message(
    const ::test::Message& message,
    const int32_t tester_id,
    int32_t & count);

} // namespace test

#endif // ProtobufMessageUtils_hpp
