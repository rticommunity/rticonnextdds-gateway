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
#include "ProtobufMessageUtils.hpp"

namespace test {

static
int32_t fv_max_collection_size = 50;

void
generate_bytes_data(std::string & bytes, const int32_t count)
{
    static const char bytes_data[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05
    };
    int32_t bytes_len = count % sizeof(bytes_data);
    if (bytes_len == 0) {
        bytes_len += 1;
    }
    bytes.assign(bytes_data, bytes_len);
}

void
generate_anothermessage(
    ::test::AnotherMessage& another_message,
    const int32_t count)
{
    another_message.Clear();

    another_message.set_req_int32(count);
    another_message.set_req_string("AnotherMessage req string " + std::to_string(count));

    if (count % 2 == 0) {
        another_message.set_req_enum(::test::HELLO);

        another_message.set_opt_int32(count);
        another_message.set_opt_string("AnotherMessage opt string " + std::to_string(count));
        another_message.set_opt_enum(::test::WORLD);
    } else {
        another_message.set_req_enum(::test::WORLD);
    }
}

void
verify_anothermessage(
    const ::test::AnotherMessage& another_message,
    const int32_t count)
{
    if (another_message.req_int32() != count) {
        throw std::runtime_error("AnotherMessage req_int32 mismatch");
    }

    if (another_message.req_string() != ("AnotherMessage req string " + std::to_string(count))) {
        throw std::runtime_error("AnotherMessage req_string mismatch");
    }

    if (count % 2 == 0) {
        if (another_message.req_enum() != ::test::HELLO) {
            throw std::runtime_error("AnotherMessage req_enum mismatch");
        }

        if (!another_message.has_opt_int32()) {
            throw std::runtime_error("AnotherMessage missing opt_int32");
        }
        if (another_message.opt_int32() != count) {
            throw std::runtime_error("AnotherMessage opt_int32 mismatch");
        }

        if (!another_message.has_opt_string()) {
            throw std::runtime_error("AnotherMessage missing opt_string");
        }
        if (another_message.opt_string() != ("AnotherMessage opt string " + std::to_string(count))) {
            throw std::runtime_error("AnotherMessage opt_string mismatch");
        }

        if (!another_message.has_opt_enum()) {
            throw std::runtime_error("AnotherMessage missing opt_enum");
        }
        if (another_message.opt_enum() != ::test::WORLD) {
            throw std::runtime_error("AnotherMessage opt_enum mismatch");
        }
    } else {
        if (another_message.req_enum() != ::test::WORLD) {
            throw std::runtime_error("AnotherMessage req_enum mismatch");
        }

        if (another_message.has_opt_int32()) {
            throw std::runtime_error("AnotherMessage unexpected opt_int32");
        }

        if (another_message.has_opt_string()) {
            throw std::runtime_error("AnotherMessage unexpected opt_string");
        }

        if (another_message.has_opt_enum()) {
            throw std::runtime_error("AnotherMessage unexpected opt_enum");
        }
    }
}

void
generate_allrequiredmessage(
    ::test::AllRequiredMessage& all_required_message,
    const int32_t count)
{
    all_required_message.Clear();

    all_required_message.set_req_int32(count);
    all_required_message.set_req_string("AllRequiredMessage req string " + std::to_string(count));
    all_required_message.set_req_enum(
        count % 2 == 0
        ? ::test::WORLD
        : ::test::HELLO
    );
}

void
verify_allrequiredmessage(
    const ::test::AllRequiredMessage& all_required_message,
    const int32_t count)
{
    if (all_required_message.req_int32() != count) {
        throw std::runtime_error("AllRequiredMessage req_int32 mismatch");
    }

    if (all_required_message.req_string() != ("AllRequiredMessage req string " + std::to_string(count))) {
        throw std::runtime_error("AllRequiredMessage req_string mismatch");
    }

    if (all_required_message.req_enum() !=
            (count % 2 == 0 ? ::test::WORLD : ::test::HELLO)) {
        throw std::runtime_error("AllRequiredMessage req_enum mismatch");
    }
}

void
generate_submessage(
    ::test::Message_SubMessage& submessage,
    const int32_t count)
{
    std::string bytes_string;
    generate_bytes_data(bytes_string, count);

    submessage.Clear();

    submessage.set_req_int32(count);
    submessage.set_req_string("Submessage req string " + std::to_string(count));
    submessage.set_req_bytes(bytes_string);

    auto & req_msg = *submessage.mutable_req_msg();
    generate_anothermessage(req_msg, count);

    if (count % 2 == 0) {
        submessage.set_req_enum(::test::HELLO);

        submessage.set_opt_int32(count);
        submessage.set_opt_string("Submessage opt string " + std::to_string(count));
        submessage.set_opt_bytes(bytes_string);
        submessage.set_opt_enum(::test::WORLD);

        auto opt_msg = submessage.mutable_opt_msg();
        generate_anothermessage(*opt_msg, count);
    } else {
        submessage.set_req_enum(::test::WORLD);
    }

    for (int i = 0; i < (count % fv_max_collection_size); i++) {
        int el_count = i + count;

        submessage.add_rep_int32(el_count);
        submessage.add_rep_string("Submessage rep string " + std::to_string(el_count));
        submessage.add_rep_bytes(bytes_string);
        submessage.add_rep_enum(::test::WORLD);

        auto rep_msg = submessage.add_rep_msg();
        generate_anothermessage(*rep_msg, el_count);
    }

    switch (count % 5) {
    case 0:
        submessage.set_oneof_int32(count);
        break;
    case 1:
        submessage.set_oneof_string("Submessage oneof string " + std::to_string(count));
        break;
    case 2:
        submessage.set_oneof_bytes(bytes_string);
        break;
    case 3:
        submessage.set_oneof_enum(::test::HELLO);
        break;
    case 4:
        {
            auto oneof_msg = submessage.mutable_oneof_msg();
            generate_anothermessage(*oneof_msg, count);
            break;
        }
    }
}

void
verify_submessage(
    const ::test::Message_SubMessage& submessage,
    const int32_t count)
{
    if (submessage.req_int32() != count) {
        throw std::runtime_error("Submessage req_int32 mismatch");
    }

    if (submessage.req_string() != ("Submessage req string " + std::to_string(count))) {
        throw std::runtime_error("Submessage req_string mismatch");
    }

    std::string expected_bytes;
    generate_bytes_data(expected_bytes, count);
    if (submessage.req_bytes() != expected_bytes) {
        throw std::runtime_error("Submessage req_bytes mismatch");
    }

    if (count % 2 == 0) {
        if (submessage.req_enum() != ::test::HELLO) {
            throw std::runtime_error("Submessage req_enum mismatch");
        }

        if (!submessage.has_opt_int32()) {
            throw std::runtime_error("Submessage missing opt_int32");
        }
        if (submessage.opt_int32() != count) {
            throw std::runtime_error("Submessage opt_int32 mismatch");
        }

        if (!submessage.has_opt_string()) {
            throw std::runtime_error("Submessage missing opt_string");
        }
        if (submessage.opt_string() != ("Submessage opt string " + std::to_string(count))) {
            throw std::runtime_error("Submessage opt_string mismatch");
        }

        if (!submessage.has_opt_bytes()) {
            throw std::runtime_error("Submessage missing opt_bytes");
        }
        if (submessage.opt_bytes() != expected_bytes) {
            throw std::runtime_error("Submessage opt_bytes mismatch");
        }

        if (!submessage.has_opt_enum()) {
            throw std::runtime_error("Submessage missing opt_enum");
        }
        if (submessage.opt_enum() != ::test::WORLD) {
            throw std::runtime_error("Submessage opt_enum mismatch");
        }

        if (!submessage.has_opt_msg()) {
            throw std::runtime_error("Submessage missing opt_msg");
        }
        verify_anothermessage(submessage.opt_msg(), count);
    } else {
        if (submessage.req_enum() != ::test::WORLD) {
            throw std::runtime_error("Submessage req_enum mismatch");
        }

        if (submessage.has_opt_int32()) {
            throw std::runtime_error("Submessage unexpected opt_int32");
        }

        if (submessage.has_opt_string()) {
            throw std::runtime_error("Submessage unexpected opt_string");
        }
        if (submessage.has_opt_bytes()) {
            throw std::runtime_error("Submessage unexpected opt_bytes");
        }
        if (submessage.has_opt_enum()) {
            throw std::runtime_error("Submessage unexpected opt_enum");
        }
        if (submessage.has_opt_msg()) {
            throw std::runtime_error("Submessage unexpected opt_msg");
        }
    }
    if (submessage.rep_int32_size() != (count % fv_max_collection_size)) {
        throw std::runtime_error("Submessage rep_int32 size mismatch");
    }
    if (submessage.rep_string_size() != (count % fv_max_collection_size)) {
        throw std::runtime_error("Submessage rep_string size mismatch");
    }
    if (submessage.rep_bytes_size() != (count % fv_max_collection_size)) {
        throw std::runtime_error("Submessage rep_bytes size mismatch");
    }
    if (submessage.rep_enum_size() != (count % fv_max_collection_size)) {
        throw std::runtime_error("Submessage rep_enum size mismatch");
    }
    if (submessage.rep_msg_size() != (count % fv_max_collection_size)) {
        throw std::runtime_error("Submessage rep_msg size mismatch");
    }
    for (int i = 0; i < (count % fv_max_collection_size); i++) {
        int el_count = i + count;

        if (submessage.rep_int32(i) != el_count) {
            throw std::runtime_error("Submessage rep_int32[" + std::to_string(i) + "] mismatch");
        }

        if (submessage.rep_string(i) != ("Submessage rep string " + std::to_string(el_count))) {
            throw std::runtime_error("Submessage rep_string[" + std::to_string(i) + "] mismatch");
        }

        if (submessage.rep_bytes(i) != expected_bytes) {
            throw std::runtime_error("Submessage rep_bytes[" + std::to_string(i) + "] mismatch");
        }

        if (submessage.rep_enum(i) != ::test::WORLD) {
            throw std::runtime_error("Submessage rep_enum[" + std::to_string(i) + "] mismatch");
        }

        verify_anothermessage(submessage.rep_msg(i), el_count);
    }
    switch (count % 5) {
    case 0:
        if (!submessage.has_oneof_int32()) {
            throw std::runtime_error("Submessage missing oneof_int32");
        }
        if (submessage.oneof_int32() != count) {
            throw std::runtime_error("Submessage oneof_int32 mismatch");
        }
        break;
    case 1:
        if (!submessage.has_oneof_string()) {
            throw std::runtime_error("Submessage missing oneof_string");
        }
        if (submessage.oneof_string() != ("Submessage oneof string " + std::to_string(count))) {
            throw std::runtime_error("Submessage oneof_string mismatch");
        }
        break;
    case 2:
        if (!submessage.has_oneof_bytes()) {
            throw std::runtime_error("Submessage missing oneof_bytes");
        }
        if (submessage.oneof_bytes() != expected_bytes) {
            throw std::runtime_error("Submessage oneof_bytes mismatch");
        }
        break;
    case 3:
        if (!submessage.has_oneof_enum()) {
            throw std::runtime_error("Submessage missing oneof_enum");
        }
        if (submessage.oneof_enum() != ::test::HELLO) {
            throw std::runtime_error("Submessage oneof_enum mismatch");
        }
        break;
    case 4:
        if (!submessage.has_oneof_msg()) {
            throw std::runtime_error("Submessage missing oneof_msg");
        }
        verify_anothermessage(submessage.oneof_msg(), count);
        break;
    }
}

void
generate_message(
    ::test::Message& message,
    const int32_t tester_id,
    const int32_t count)
{
    std::string bytes_string;
    generate_bytes_data(bytes_string, count);

    message.Clear();

    message.set_key_primitive(tester_id);
    message.set_key_string("Key string " + std::to_string(tester_id));
    message.set_key_enum(
        tester_id % 2 == 0
        ? ::test::Message_SubEnum_WORLD
        : ::test::Message_SubEnum_HELLO
    );

    auto key_msg = message.mutable_key_msg();
    generate_allrequiredmessage(*key_msg, tester_id);

    message.set_req_int32(-1001 + count);
    message.set_req_uint32(1001 + count);
    message.set_req_sint32(-1001 + count);
    message.set_req_fixed32(1001 + count);
    message.set_req_sfixed32(-1001 + count);
    message.set_req_int64(-1000001 + count);
    message.set_req_uint64(1000001 + count);
    message.set_req_sint64(-1000001 + count);
    message.set_req_fixed64(1000001 + count);
    message.set_req_sfixed64(-1000001 + count);
    message.set_req_float(-1001.100098f + count);
    message.set_req_double(1001.100098 + count);
    message.set_req_string("Req string " + std::to_string(count));
    message.set_req_bytes(bytes_string);
    message.set_req_bool((count % 2) == 0);
    message.set_req_enum(
        count % 2 == 0
        ? ::test::Message_SubEnum_WORLD
        : ::test::Message_SubEnum_HELLO
    );

    auto req_msg = message.mutable_req_msg();
    generate_submessage(*req_msg, count);

    auto group_req = message.mutable_groupreq();
    group_req->set_group_req_string("Group req string " + std::to_string(count));

    if (count % 2 == 0) {
        message.set_opt_int32(-1001 + count);
        message.set_opt_sint32(-1001 + count);
        message.set_opt_sfixed32(-1001 + count);
        message.set_opt_uint64(1000001 + count);
        message.set_opt_fixed64(1000001 + count);
        message.set_opt_float(-1001.100098f + count);
        message.set_opt_string("Opt string " + std::to_string(count));
        message.set_opt_bool((count % 3) == 0);
        message.set_opt_enum(
            count % 3 == 0
            ? ::test::Message_SubEnum_WORLD
            : ::test::Message_SubEnum_HELLO
        );

        for (int i = 0; i < (count % fv_max_collection_size); i++) {
            int el_count = i + count;

            message.add_rep_uint32(1001 + el_count);
            message.add_rep_fixed32(1001 + el_count);
            message.add_rep_int64(-1000001 + el_count);
            message.add_rep_sint64(-1000001 + el_count);
            message.add_rep_sfixed64(-1000001 + el_count);
            message.add_rep_double(1001.100098 + el_count);
            message.add_rep_bytes(bytes_string);
            auto rep_msg = message.add_rep_msg();
            generate_submessage(*rep_msg, el_count);

            std::string key_string = "Map key " + std::to_string(el_count);
            
            (*message.mutable_map_string_primitive())[key_string] = -1001 + el_count;
            (*message.mutable_map_string_enum())[key_string] =
                el_count % 2 == 0
                ? ::test::HELLO
                : ::test::WORLD;
            auto & val_string_msg = (*message.mutable_map_string_msg())[key_string];
            generate_submessage(val_string_msg, el_count);
            (*message.mutable_map_primitive_bool())[-1001 + el_count] = (el_count % 2) == 0;            

            auto group_rep = message.add_grouprep();
            group_rep->set_group_rep_string("Group rep string " + std::to_string(el_count));
        }
    } else {
        message.set_opt_uint32(1001 + count);
        message.set_opt_fixed32(1001 + count);
        message.set_opt_int64(-1000001 + count);
        message.set_opt_sint64(-1000001 + count);
        message.set_opt_sfixed64(-1000001 + count);
        message.set_opt_double(1001.100098 + count);
        message.set_opt_bytes(bytes_string);
        auto opt_msg = message.mutable_opt_msg();
        generate_submessage(*opt_msg, count);

        for (int i = 0; i < (count % fv_max_collection_size); i++) {
            int el_count = i + count;

            message.add_rep_int32(el_count);
            message.add_rep_sint32(el_count);
            message.add_rep_sfixed32(el_count);
            message.add_rep_uint64(1000001 + el_count);
            message.add_rep_fixed64(1000001 + el_count);
            message.add_rep_float(-1001.100098f + el_count);
            message.add_rep_string("Rep string " + std::to_string(el_count));
            message.add_rep_bool((el_count % 2) == 0);
            message.add_rep_enum(
                el_count % 2 == 0
                ? ::test::Message_SubEnum_WORLD
                : ::test::Message_SubEnum_HELLO
            );

            std::string val_string = "Map value " + std::to_string(el_count);
            (*message.mutable_map_primitive_string())[-1001 + el_count] = val_string;
            (*message.mutable_map_primitive_enum())[-1000001 + el_count] =
                el_count % 2 == 0
                ? ::test::Message_SubEnum_HELLO
                : ::test::Message_SubEnum_WORLD;
            auto & val_primitive_msg = (*message.mutable_map_primitive_msg())[el_count];
            generate_anothermessage(val_primitive_msg, el_count);
        }

        for (int i = 0; i < (count % 2); i++) {
            (*message.mutable_map_bool_primitive())[((count % 2) == 0)] = 1001.100098 + count;
        }

        auto group_opt = message.mutable_groupopt();
        group_opt->set_group_opt_string("Group opt string " + std::to_string(count));
    }

    switch (count % 6) {
    case 0:
        message.set_oneof_primitive(count);
        break;
    case 1:
        message.set_oneof_string("Oneof string " + std::to_string(count));
        break;
    case 2:
        message.set_oneof_bytes(bytes_string);
        break;
    case 3:
        message.set_oneof_bool((count % 2) == 0);
        break;
    case 4:
        message.set_oneof_enum(
            count % 2 == 0
            ? ::test::Message_SubEnum_WORLD
            : ::test::Message_SubEnum_HELLO
        );
        break;
    case 5:
        {
            auto oneof_msg = message.mutable_oneof_msg();
            generate_submessage(*oneof_msg, count);
            break;
        }
    }
}

void
verify_message(
    const ::test::Message& message,
    const int32_t tester_id,
    int32_t & count)
{
    if (message.key_primitive() != tester_id) {
        throw std::runtime_error("Message key_primitive mismatch");
    }

    if (message.key_string() != ("Key string " + std::to_string(tester_id))) {
        std::cout << "Expected: " << ("Key string " + std::to_string(tester_id)) << std::endl;
        std::cout << "Actual:   " << message.key_string() << std::endl;
        throw std::runtime_error("Message key_string mismatch");
    }

    if (message.key_enum() !=
            (tester_id % 2 == 0 ? ::test::Message_SubEnum_WORLD : ::test::Message_SubEnum_HELLO)) {
        throw std::runtime_error("Message key_enum mismatch");
    }

    verify_allrequiredmessage(message.key_msg(), tester_id);

    count = message.req_int32() + 1001;

    if (message.req_int32() != (-1001 + count)) {
        throw std::runtime_error("Message req_int32 mismatch");
    }
    if (message.req_uint32() != (1001 + count)) {
        throw std::runtime_error("Message req_uint32 mismatch");
    }
    if (message.req_sint32() != (-1001 + count)) {
        throw std::runtime_error("Message req_sint32 mismatch");
    }
    if (message.req_fixed32() != (1001 + count)) {
        throw std::runtime_error("Message req_fixed32 mismatch");
    }
    if (message.req_sfixed32() != (-1001 + count)) {
        throw std::runtime_error("Message req_sfixed32 mismatch");
    }
    if (message.req_int64() != (-1000001 + count)) {
        throw std::runtime_error("Message req_int64 mismatch");
    }
    if (message.req_uint64() != (1000001 + count)) {
        throw std::runtime_error("Message req_uint64 mismatch");
    }
    if (message.req_sint64() != (-1000001 + count)) {
        throw std::runtime_error("Message req_sint64 mismatch");
    }
    if (message.req_fixed64() != (1000001 + count)) {
        throw std::runtime_error("Message req_fixed64 mismatch");
    }
    if (message.req_sfixed64() != (-1000001 + count)) {
        throw std::runtime_error("Message req_sfixed64 mismatch");
    }
    if (message.req_float() != (-1001.100098f + count)) {
        throw std::runtime_error("Message req_float mismatch");
    }
    if (message.req_double() != (1001.100098 + count)) {
        throw std::runtime_error("Message req_double mismatch");
    }
    if (message.req_string() != ("Req string " + std::to_string(count))) {
        throw std::runtime_error("Message req_string mismatch");
    }
    std::string expected_bytes;
    generate_bytes_data(expected_bytes, count);
    if (message.req_bytes() != expected_bytes) {
        throw std::runtime_error("Message req_bytes mismatch");
    }
    if (message.req_bool() != ((count % 2) == 0)) {
        throw std::runtime_error("Message req_bool mismatch");
    }
    if (message.req_enum() !=
            (count % 2 == 0 ? ::test::Message_SubEnum_WORLD : ::test::Message_SubEnum_HELLO)) {
        throw std::runtime_error("Message req_enum mismatch");
    }
    verify_submessage(message.req_msg(), count);

    if (count % 2 == 0) {
        if (!message.has_opt_int32()) {
            throw std::runtime_error("Message missing opt_int32");
        }
        if (message.opt_int32() != (-1001 + count)) {
            throw std::runtime_error("Message opt_int32 mismatch");
        }

        if (!message.has_opt_sint32()) {
            throw std::runtime_error("Message missing opt_sint32");
        }
        if (message.opt_sint32() != (-1001 + count)) {
            throw std::runtime_error("Message opt_sint32 mismatch");
        }

        if (!message.has_opt_sfixed32()) {
            throw std::runtime_error("Message missing opt_sfixed32");
        }
        if (message.opt_sfixed32() != (-1001 + count)) {
            throw std::runtime_error("Message opt_sfixed32 mismatch");
        }

        if (!message.has_opt_uint64()) {
            throw std::runtime_error("Message missing opt_uint64");
        }
        if (message.opt_uint64() != (1000001 + count)) {
            throw std::runtime_error("Message opt_uint64 mismatch");
        }

        if (!message.has_opt_fixed64()) {
            throw std::runtime_error("Message missing opt_fixed64");
        }
        if (message.opt_fixed64() != (1000001 + count)) {
            throw std::runtime_error("Message opt_fixed64 mismatch");
        }

        if (!message.has_opt_float()) {
            throw std::runtime_error("Message missing opt_float");
        }
        if (message.opt_float() != (-1001.100098f + count)) {
            throw std::runtime_error("Message opt_float mismatch");
        }

        if (!message.has_opt_string()) {
            throw std::runtime_error("Message missing opt_string");
        }
        if (message.opt_string() != ("Opt string " + std::to_string(count))) {
            throw std::runtime_error("Message opt_string mismatch");
        }

        if (!message.has_opt_bool()) {
            throw std::runtime_error("Message missing opt_bool");
        }
        if (message.opt_bool() != ((count % 3) == 0)) {
            throw std::runtime_error("Message opt_bool mismatch");
        }

        if (!message.has_opt_enum()) {
            throw std::runtime_error("Message missing opt_enum");
        }
        if (message.opt_enum() !=
                (count % 3 == 0 ? ::test::Message_SubEnum_WORLD : ::test::Message_SubEnum_HELLO)) {
            throw std::runtime_error("Message opt_enum mismatch");
        }
        if (message.rep_uint32_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_uint32 size mismatch");
        }
        if (message.rep_fixed32_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_fixed32 size mismatch");
        }
        if (message.rep_int64_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_int64 size mismatch");
        }
        if (message.rep_sint64_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_sint64 size mismatch");
        }
        if (message.rep_sfixed64_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_sfixed64 size mismatch");
        }
        if (message.rep_double_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_double size mismatch");
        }
        if (message.rep_bytes_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_bytes size mismatch");
        }
        if (message.rep_msg_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_msg size mismatch");
        }
        if (message.map_string_primitive_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message map_string_primitive size mismatch");
        }
        if (message.map_string_enum_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message map_string_enum size mismatch");
        }
        if (message.map_primitive_bool_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message map_primitive_bool size mismatch");
        }
        if (message.grouprep_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message group_rep size mismatch");
        }
        for (int i = 0; i < (count % fv_max_collection_size); i++) {
            int el_count = i + count;
            if (message.rep_uint32(i) != (1001 + el_count)) {
                throw std::runtime_error("Message rep_uint32[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_fixed32(i) != (1001 + el_count)) {
                throw std::runtime_error("Message rep_fixed32[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_int64(i) != (-1000001 + el_count)) {
                throw std::runtime_error("Message rep_int64[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_sint64(i) != (-1000001 + el_count)) {
                throw std::runtime_error("Message rep_sint64[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_sfixed64(i) != (-1000001 + el_count)) {
                throw std::runtime_error("Message rep_sfixed64[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_double(i) != (1001.100098 + el_count)) {
                throw std::runtime_error("Message rep_double[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_bytes(i) != expected_bytes) {
                throw std::runtime_error("Message rep_bytes[" + std::to_string(i) + "] mismatch");
            }
            verify_submessage(message.rep_msg(i), el_count);
            std::string key_string = "Map key " + std::to_string(el_count);
            auto it_primitive = message.map_string_primitive().find(key_string);
            if (it_primitive == message.map_string_primitive().end()) {
                throw std::runtime_error("Message missing map_string_primitive[" + key_string + "]");
            }
            if (it_primitive->second != (-1001 + el_count)) {
                throw std::runtime_error("Message map_string_primitive[" + key_string + "] mismatch");
            }
            auto it_enum = message.map_string_enum().find(key_string);
            if (it_enum == message.map_string_enum().end()) {
                throw std::runtime_error("Message missing map_string_enum[" + key_string + "]");
            }
            if (it_enum->second !=
                    (el_count % 2 == 0 ? ::test::HELLO : ::test::WORLD)) {
                throw std::runtime_error("Message map_string_enum[" + key_string + "] mismatch");
            }
            auto it_bool = message.map_primitive_bool().find(-1001 + el_count);
            if (it_bool == message.map_primitive_bool().end()) {
                throw std::runtime_error("Message missing map_primitive_bool[" + std::to_string(-1001 + el_count) + "]");
            }
            if (it_bool->second != ((el_count % 2) == 0)) {
                throw std::runtime_error("Message map_primitive_bool[" + std::to_string(-1001 + el_count) + "] mismatch");
            }
            auto group_rep = message.grouprep(i);
            if (group_rep.group_rep_string() != ("Group rep string " + std::to_string(el_count))) {
                throw std::runtime_error("Message group_rep[" + std::to_string(i) + "] group_rep_string mismatch");
            }
        }
        if (!message.has_groupreq()) {
            throw std::runtime_error("Message missing group_req");
        }
        if (message.groupreq().group_req_string() != ("Group req string " + std::to_string(count))) {
            throw std::runtime_error("Message group_req.group_req_string mismatch");
        }
    } else {
        if (!message.has_opt_uint32()) {
            throw std::runtime_error("Message missing opt_uint32");
        }
        if (message.opt_uint32() != (1001 + count)) {
            throw std::runtime_error("Message opt_uint32 mismatch");
        }
        if (!message.has_opt_fixed32()) {
            throw std::runtime_error("Message missing opt_fixed32");
        }
        if (message.opt_fixed32() != (1001 + count)) {
            throw std::runtime_error("Message opt_fixed32 mismatch");
        }
        if (!message.has_opt_int64()) {
            throw std::runtime_error("Message missing opt_int64");
        }
        if (message.opt_int64() != (-1000001 + count)) {
            throw std::runtime_error("Message opt_int64 mismatch");
        }
        if (!message.has_opt_sint64()) {
            throw std::runtime_error("Message missing opt_sint64");
        }
        if (message.opt_sint64() != (-1000001 + count)) {
            throw std::runtime_error("Message opt_sint64 mismatch");
        }
        if (!message.has_opt_sfixed64()) {
            throw std::runtime_error("Message missing opt_sfixed64");
        }
        if (message.opt_sfixed64() != (-1000001 + count)) {
            throw std::runtime_error("Message opt_sfixed64 mismatch");
        }
        if (!message.has_opt_double()) {
            throw std::runtime_error("Message missing opt_double");
        }
        if (message.opt_double() != (1001.100098 + count)) {
            throw std::runtime_error("Message opt_double mismatch");
        }
        if (!message.has_opt_bytes()) {
            throw std::runtime_error("Message missing opt_bytes");
        }
        if (message.opt_bytes() != expected_bytes) {
            throw std::runtime_error("Message opt_bytes mismatch");
        }
        if (!message.has_opt_msg()) {
            throw std::runtime_error("Message missing opt_msg");
        }
        verify_submessage(message.opt_msg(), count);
        if (message.rep_int32_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_int32 size mismatch");
        }
        if (message.rep_sint32_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_sint32 size mismatch");
        }
        if (message.rep_sfixed32_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_sfixed32 size mismatch");
        }
        if (message.rep_uint64_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_uint64 size mismatch");
        }
        if (message.rep_fixed64_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_fixed64 size mismatch");
        }
        if (message.rep_float_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_float size mismatch");
        }
        if (message.rep_string_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_string size mismatch");
        }
        if (message.rep_bool_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_bool size mismatch");
        }
        if (message.rep_enum_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message rep_enum size mismatch");
        }
        if (message.map_primitive_string_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message map_primitive_string size mismatch");
        }
        if (message.map_primitive_enum_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message map_primitive_enum size mismatch");
        }
        if (message.map_primitive_msg_size() != (count % fv_max_collection_size)) {
            throw std::runtime_error("Message map_primitive_msg size mismatch");
        }
        for (int i = 0; i < (count % fv_max_collection_size); i++) {
            int el_count = i + count;
            if (message.rep_int32(i) != el_count) {
                throw std::runtime_error("Message rep_int32[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_sint32(i) != el_count) {
                throw std::runtime_error("Message rep_sint32[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_sfixed32(i) != el_count) {
                throw std::runtime_error("Message rep_sfixed32[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_uint64(i) != (1000001 + el_count)) {
                throw std::runtime_error("Message rep_uint64[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_fixed64(i) != (1000001 + el_count)) {
                throw std::runtime_error("Message rep_fixed64[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_float(i) != (-1001.100098f + el_count)) {
                throw std::runtime_error("Message rep_float[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_string(i) != ("Rep string " + std::to_string(el_count))) {
                throw std::runtime_error("Message rep_string[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_bool(i) != ((el_count % 2) == 0)) {
                throw std::runtime_error("Message rep_bool[" + std::to_string(i) + "] mismatch");
            }
            if (message.rep_enum(i) !=
                    (el_count % 2 == 0 ? ::test::Message_SubEnum_WORLD : ::test::Message_SubEnum_HELLO)) {
                throw std::runtime_error("Message rep_enum[" + std::to_string(i) + "] mismatch");
            }
            std::string val_string = "Map value " + std::to_string(el_count);
            auto it_string = message.map_primitive_string().find(-1001 + el_count);
            if (it_string == message.map_primitive_string().end()) {
                throw std::runtime_error("Message missing map_primitive_string[" + std::to_string(-1001 + el_count) + "]");
            }
            if (it_string->second != val_string) {
                throw std::runtime_error("Message map_primitive_string[" + std::to_string(-1001 + el_count) + "] mismatch");
            }
            auto it_enum = message.map_primitive_enum().find(-1000001 + el_count);
            if (it_enum == message.map_primitive_enum().end()) {
                throw std::runtime_error("Message missing map_primitive_enum[" + std::to_string(-1000001 + el_count) + "]");
            }
            if (it_enum->second !=
                    (el_count % 2 == 0 ? ::test::Message_SubEnum_HELLO : ::test::Message_SubEnum_WORLD)) {
                throw std::runtime_error("Message map_primitive_enum[" + std::to_string(-1000001 + el_count) + "] mismatch");
            }
            auto it_msg = message.map_primitive_msg().find(el_count);
            if (it_msg == message.map_primitive_msg().end()) {
                throw std::runtime_error("Message missing map_primitive_msg[" + std::to_string(el_count) + "]");
            }
            verify_anothermessage(it_msg->second, el_count);
        }
        if (message.map_bool_primitive_size() != (count % 2)) {
            throw std::runtime_error("Message map_bool_primitive size mismatch");
        }
        for (int i = 0; i < (count % 2); i++) {
            auto it_bool = message.map_bool_primitive().find((count % 2) == 0);
            if (it_bool == message.map_bool_primitive().end()) {
                throw std::runtime_error("Message missing map_bool_primitive[" + std::to_string((count % 2) == 0) + "]");
            }
            if (it_bool->second != (1001.100098 + count)) {
                throw std::runtime_error("Message map_bool_primitive[" + std::to_string((count % 2) == 0) + "] mismatch");
            }
        }
        if (!message.has_groupopt()) {
            throw std::runtime_error("Message missing group_opt");
        }
        if (message.groupopt().group_opt_string() != ("Group opt string " + std::to_string(count))) {
            throw std::runtime_error("Message group_opt.group_opt_string mismatch");
        }
    }
    switch (count % 6) {
    case 0:
        if (!message.has_oneof_primitive()) {
            throw std::runtime_error("Message missing oneof_primitive");
        }
        if (message.oneof_primitive() != count) {
            throw std::runtime_error("Message oneof_primitive mismatch");
        }
        break;
    case 1:
        if (!message.has_oneof_string()) {
            throw std::runtime_error("Message missing oneof_string");
        }
        if (message.oneof_string() != ("Oneof string " + std::to_string(count))) {
            throw std::runtime_error("Message oneof_string mismatch");
        }
        break;
    case 2:
        if (!message.has_oneof_bytes()) {
            throw std::runtime_error("Message missing oneof_bytes");
        }
        if (message.oneof_bytes() != expected_bytes) {
            throw std::runtime_error("Message oneof_bytes mismatch");
        }
        break;
    case 3:
        if (!message.has_oneof_bool()) {
            throw std::runtime_error("Message missing oneof_bool");
        }
        if (message.oneof_bool() != ((count % 2) == 0)) {
            throw std::runtime_error("Message oneof_bool mismatch");
        }
        break;
    case 4:
        if (!message.has_oneof_enum()) {
            throw std::runtime_error("Message missing oneof_enum");
        }
        if (message.oneof_enum() !=
                (count % 2 == 0 ? ::test::Message_SubEnum_WORLD : ::test::Message_SubEnum_HELLO)) {
            throw std::runtime_error("Message oneof_enum mismatch");
        }
        break;
    case 5:
        if (!message.has_oneof_msg()) {
            throw std::runtime_error("Message missing oneof_msg");
        }
        verify_submessage(message.oneof_msg(), count);
        break;
    }
    
}

} // namespace test
