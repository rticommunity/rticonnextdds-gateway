#include "ndds/ndds_c.h"
#include "rdkafka.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define REQUIRE(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); exit(1); \
} } while (0)

static void pause_ms(int ms)
{
    struct DDS_Duration_t delay = {ms / 1000, (ms % 1000) * 1000000};
    NDDS_Utility_sleep(&delay);
}

static void print_hex(const void *value, size_t length)
{
    const unsigned char *bytes = value;
    size_t i;
    printf("{\"hex\":\"");
    for (i = 0; i < length; i++) printf("%02x", bytes[i]);
    puts("\"}");
    fflush(stdout);
}

static size_t binary_value(unsigned char *value, int id)
{
    const unsigned char wire[] = {0x0a, 4, (unsigned char)id, 0, 0xff, (id % 2 == 0 ? 0 : 0x80)};
    memcpy(value, wire, sizeof(wire));
    return id == 0 ? 0 : sizeof(wire);
}

static void shape_json(char *value, size_t capacity, int id)
{
    snprintf(value, capacity, "{\"color\":\"BLUE\",\"x\":%d,\"y\":%d,\"shapesize\":30}", id, id * 2);
}

static void configure(rd_kafka_conf_t *conf, const char *name, const char *value)
{
    char error[512];
    REQUIRE(rd_kafka_conf_set(conf, name, value, error, sizeof(error)) == RD_KAFKA_CONF_OK);
}

static void kafka_io(const char *mode, const char *bootstrap, const char *topic, int count)
{
    int publish = strstr(mode, "pub") != NULL;
    int binary = strstr(mode, "bytes") != NULL;
    char error[512], json[256];
    unsigned char wire[16];
    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    rd_kafka_t *client;
    int i = 0;
    time_t deadline = time(NULL) + 35;
    configure(conf, "bootstrap.servers", bootstrap);
    configure(conf, "socket.timeout.ms", "10000");
    if (!publish) {
        configure(conf, "group.id", topic);
        configure(conf, "auto.offset.reset", "earliest");
        configure(conf, "enable.auto.commit", "false");
    }
    client = rd_kafka_new(publish ? RD_KAFKA_PRODUCER : RD_KAFKA_CONSUMER, conf, error, sizeof(error));
    REQUIRE(client != NULL);
    if (publish) {
        for (i = 0; i < count; i++) {
            size_t length;
            void *value;
            if (binary) {
                length = binary_value(wire, i);
                value = wire;
            } else {
                shape_json(json, sizeof(json), i);
                length = strlen(json);
                value = json;
            }
            REQUIRE(rd_kafka_producev(client, RD_KAFKA_V_TOPIC(topic),
                    RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY), RD_KAFKA_V_VALUE(value, length),
                    RD_KAFKA_V_END) == RD_KAFKA_RESP_ERR_NO_ERROR);
            rd_kafka_poll(client, 0);
        }
        REQUIRE(rd_kafka_flush(client, 15000) == RD_KAFKA_RESP_ERR_NO_ERROR);
    } else {
        rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
        rd_kafka_topic_partition_list_add(subscription, topic, RD_KAFKA_PARTITION_UA);
        REQUIRE(rd_kafka_subscribe(client, subscription) == RD_KAFKA_RESP_ERR_NO_ERROR);
        rd_kafka_topic_partition_list_destroy(subscription);
        while (i < count && time(NULL) < deadline) {
            rd_kafka_message_t *message = rd_kafka_consumer_poll(client, 200);
            if (!message) continue;
            if (!message->err) {
                print_hex(message->payload, message->len);
                i++;
            } else {
                fprintf(stderr, "Kafka consumer: %s\n", rd_kafka_message_errstr(message));
            }
            rd_kafka_message_destroy(message);
        }
        REQUIRE(rd_kafka_consumer_close(client) == RD_KAFKA_RESP_ERR_NO_ERROR);
    }
    rd_kafka_destroy(client);
    REQUIRE(i == count);
}

static DDS_TypeCode *make_type(int binary)
{
    DDS_TypeCodeFactory *factory = DDS_TypeCodeFactory_get_instance();
    DDS_ExceptionCode_t error;
    struct DDS_StructMemberSeq members = DDS_SEQUENCE_INITIALIZER;
    DDS_TypeCode *type, *member;
    const DDS_TypeCode *integer = DDS_TypeCodeFactory_get_primitive_tc(factory, DDS_TK_LONG);
    type = DDS_TypeCodeFactory_create_struct_tc(factory, binary ? "MessagePayload" : "ShapeType", &members, &error);
    REQUIRE(type && error == DDS_NO_EXCEPTION_CODE);
    if (binary) {
        DDS_TypeCode *outer;
        member = DDS_TypeCodeFactory_create_sequence_tc(factory, DDS_LENGTH_UNLIMITED,
                DDS_TypeCodeFactory_get_primitive_tc(factory, DDS_TK_OCTET), &error);
        REQUIRE(member && error == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCode_add_member(type, "data", DDS_TYPECODE_MEMBER_ID_INVALID, member,
                DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, &error);
        REQUIRE(error == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCodeFactory_delete_tc(factory, member, &error);
        outer = DDS_TypeCodeFactory_create_struct_tc(factory, "Message", &members, &error);
        REQUIRE(outer && error == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCode_add_member(outer, "payload", DDS_TYPECODE_MEMBER_ID_INVALID, type,
                DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, &error);
        REQUIRE(error == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCodeFactory_delete_tc(factory, type, &error);
        type = outer;
    } else {
        member = DDS_TypeCodeFactory_create_string_tc(factory, 128, &error);
        REQUIRE(member && error == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCode_add_member(type, "color", DDS_TYPECODE_MEMBER_ID_INVALID, member,
                DDS_TYPECODE_KEY_MEMBER, &error);
        REQUIRE(error == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCodeFactory_delete_tc(factory, member, &error);
        DDS_TypeCode_add_member(type, "x", DDS_TYPECODE_MEMBER_ID_INVALID, integer, DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, &error);
        REQUIRE(error == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCode_add_member(type, "y", DDS_TYPECODE_MEMBER_ID_INVALID, integer, DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, &error);
        REQUIRE(error == DDS_NO_EXCEPTION_CODE);
        DDS_TypeCode_add_member(type, "shapesize", DDS_TYPECODE_MEMBER_ID_INVALID, integer, DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, &error);
        REQUIRE(error == DDS_NO_EXCEPTION_CODE);
    }
    DDS_StructMemberSeq_finalize(&members);
    return type;
}

static void dds_io(const char *mode, int domain, const char *topic_name, int count)
{
    int publish = strstr(mode, "pub") != NULL;
    int binary = strstr(mode, "bytes") != NULL;
    int i = 0;
    time_t deadline = time(NULL) + 35;
    DDS_TypeCode *type = make_type(binary);
    DDS_ExceptionCode_t error;
    struct DDS_DynamicDataTypeProperty_t type_properties = DDS_DynamicDataTypeProperty_t_INITIALIZER;
    struct DDS_DynamicDataTypeSupport *support = DDS_DynamicDataTypeSupport_new(type, &type_properties);
    const char *type_name = binary ? "RTI::Kafka::Message" : "ShapeType";
    struct DDS_DomainParticipantQos participant_qos = DDS_DomainParticipantQos_INITIALIZER;
    DDS_DomainParticipant *participant;
    DDS_Topic *topic;
    REQUIRE(DDS_DomainParticipantFactory_get_default_participant_qos(
            DDS_DomainParticipantFactory_get_instance(), &participant_qos) == DDS_RETCODE_OK);
    participant_qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4;
    REQUIRE(DDS_StringSeq_set_length(&participant_qos.discovery.initial_peers, 1));
    REQUIRE(DDS_String_replace(DDS_StringSeq_get_reference(&participant_qos.discovery.initial_peers, 0), "127.0.0.1"));
    REQUIRE(DDS_StringSeq_set_length(&participant_qos.discovery.multicast_receive_addresses, 0));
    REQUIRE(DDS_PropertyQosPolicyHelper_add_property(&participant_qos.property,
            "dds.transport.UDPv4.builtin.parent.allow_interfaces", "127.0.0.1",
            DDS_BOOLEAN_FALSE) == DDS_RETCODE_OK);
    participant = DDS_DomainParticipantFactory_create_participant(
            DDS_DomainParticipantFactory_get_instance(), domain,
            &participant_qos, NULL, DDS_STATUS_MASK_NONE);
    DDS_DomainParticipantQos_finalize(&participant_qos);
    REQUIRE(participant && support);
    REQUIRE(DDS_DynamicDataTypeSupport_register_type(support, participant, type_name) == DDS_RETCODE_OK);
    topic = DDS_DomainParticipant_create_topic(participant, topic_name, type_name,
            &DDS_TOPIC_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
    REQUIRE(topic);
    if (publish) {
        DDS_Publisher *publisher = DDS_DomainParticipant_create_publisher(participant,
                &DDS_PUBLISHER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
        struct DDS_DataWriterQos qos = DDS_DataWriterQos_INITIALIZER;
        DDS_DataWriter *base;
        DDS_DynamicDataWriter *writer;
        DDS_DynamicData *sample = DDS_DynamicDataTypeSupport_create_data(support);
        struct DDS_PublicationMatchedStatus matched = DDS_PublicationMatchedStatus_INITIALIZER;
        REQUIRE(publisher && sample);
        REQUIRE(DDS_Publisher_get_default_datawriter_qos(publisher, &qos) == DDS_RETCODE_OK);
        qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        base = DDS_Publisher_create_datawriter(publisher, topic, &qos, NULL, DDS_STATUS_MASK_NONE);
        REQUIRE(base);
        DDS_DataWriterQos_finalize(&qos);
        writer = DDS_DynamicDataWriter_narrow(base);
        do {
            REQUIRE(DDS_DataWriter_get_publication_matched_status(base, &matched) == DDS_RETCODE_OK);
            if (matched.current_count > 0) break;
            pause_ms(100);
        } while (time(NULL) < deadline);
        if (matched.current_count == 0) {
            struct DDS_InstanceHandleSeq peers = DDS_SEQUENCE_INITIALIZER;
            struct DDS_OfferedIncompatibleQosStatus incompatible = DDS_OfferedIncompatibleQosStatus_INITIALIZER;
            DDS_DomainParticipant_get_discovered_participants(participant, &peers);
            DDS_DataWriter_get_offered_incompatible_qos_status(base, &incompatible);
            fprintf(stderr, "DDS discovery: %d peers, incompatible QoS count %d, type extensibility %d\n",
                    DDS_InstanceHandleSeq_get_length(&peers), incompatible.total_count,
                    DDS_TypeCode_extensibility_kind(type, &error));
            DDS_InstanceHandleSeq_finalize(&peers);
        }
        REQUIRE(matched.current_count > 0);
        for (i = 0; i < count; i++) {
            if (binary) {
                unsigned char wire[16];
                struct DDS_OctetSeq value = DDS_SEQUENCE_INITIALIZER;
                size_t length = binary_value(wire, i);
                REQUIRE(DDS_OctetSeq_set_length(&value, (DDS_Long)length));
                if (length) memcpy(DDS_OctetSeq_get_contiguous_buffer(&value), wire, length);
                REQUIRE(DDS_DynamicData_set_octet_seq(sample, "payload.data",
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, &value) == DDS_RETCODE_OK);
                DDS_OctetSeq_finalize(&value);
            } else {
                char json[256];
                shape_json(json, sizeof(json), i);
                REQUIRE(DDS_DynamicDataFormatter_from_json(sample, json) == DDS_RETCODE_OK);
            }
            REQUIRE(DDS_DynamicDataWriter_write(writer, sample, &DDS_HANDLE_NIL) == DDS_RETCODE_OK);
            pause_ms(20);
        }
        {
            struct DDS_Duration_t timeout = {10, 0};
            REQUIRE(DDS_DataWriter_wait_for_acknowledgments(base, &timeout) == DDS_RETCODE_OK);
        }
        DDS_DynamicDataTypeSupport_delete_data(support, sample);
    } else {
        DDS_Subscriber *subscriber = DDS_DomainParticipant_create_subscriber(participant,
                &DDS_SUBSCRIBER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
        struct DDS_DataReaderQos qos = DDS_DataReaderQos_INITIALIZER;
        DDS_DataReader *base;
        DDS_DynamicDataReader *reader;
        struct DDS_DynamicDataSeq data = DDS_SEQUENCE_INITIALIZER;
        struct DDS_SampleInfoSeq infos = DDS_SEQUENCE_INITIALIZER;
        REQUIRE(subscriber);
        REQUIRE(DDS_Subscriber_get_default_datareader_qos(subscriber, &qos) == DDS_RETCODE_OK);
        qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        base = DDS_Subscriber_create_datareader(subscriber, DDS_Topic_as_topicdescription(topic),
                &qos, NULL, DDS_STATUS_MASK_NONE);
        REQUIRE(base);
        DDS_DataReaderQos_finalize(&qos);
        reader = DDS_DynamicDataReader_narrow(base);
        {
            struct DDS_SubscriptionMatchedStatus matched = DDS_SubscriptionMatchedStatus_INITIALIZER;
            do {
                REQUIRE(DDS_DataReader_get_subscription_matched_status(base, &matched) == DDS_RETCODE_OK);
                if (matched.current_count > 0) break;
                pause_ms(100);
            } while (time(NULL) < deadline);
            REQUIRE(matched.current_count > 0);
        }
        puts("{\"ready\":true}"); fflush(stdout);
        while (i < count && time(NULL) < deadline) {
            DDS_ReturnCode_t result = DDS_DynamicDataReader_take(reader, &data, &infos,
                    DDS_LENGTH_UNLIMITED, DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
            if (result == DDS_RETCODE_NO_DATA) { pause_ms(20); continue; }
            REQUIRE(result == DDS_RETCODE_OK);
            {
                int n;
                for (n = 0; n < DDS_DynamicDataSeq_get_length(&data); n++) {
                    DDS_DynamicData *sample = DDS_DynamicDataSeq_get_reference(&data, n);
                    if (!DDS_SampleInfoSeq_get_reference(&infos, n)->valid_data) continue;
                    if (binary) {
                        struct DDS_OctetSeq value = DDS_SEQUENCE_INITIALIZER;
                        REQUIRE(DDS_DynamicData_get_octet_seq(sample, &value, "payload.data",
                                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED) == DDS_RETCODE_OK);
                        print_hex(DDS_OctetSeq_get_contiguous_buffer(&value), DDS_OctetSeq_get_length(&value));
                        DDS_OctetSeq_finalize(&value);
                    } else {
                        char json[1024];
                        DDS_UnsignedLong capacity = sizeof(json);
                        REQUIRE(DDS_DynamicDataFormatter_to_json(sample, json, &capacity, 0) == DDS_RETCODE_OK);
                        puts(json); fflush(stdout);
                    }
                    i++;
                }
            }
            REQUIRE(DDS_DynamicDataReader_return_loan(reader, &data, &infos) == DDS_RETCODE_OK);
        }
        DDS_DynamicDataSeq_finalize(&data);
        DDS_SampleInfoSeq_finalize(&infos);
    }
    REQUIRE(DDS_DomainParticipant_delete_contained_entities(participant) == DDS_RETCODE_OK);
    REQUIRE(DDS_DomainParticipantFactory_delete_participant(
            DDS_DomainParticipantFactory_get_instance(), participant) == DDS_RETCODE_OK);
    DDS_DynamicDataTypeSupport_delete(support);
    DDS_TypeCodeFactory_delete_tc(DDS_TypeCodeFactory_get_instance(), type, &error);
    DDS_DomainParticipantFactory_finalize_instance();
    REQUIRE(i == count);
}

int main(int argc, char **argv)
{
    REQUIRE(argc == 6);
    if (strncmp(argv[1], "dds-", 4) == 0) {
        struct DDS_DomainParticipantFactoryQos qos = DDS_DomainParticipantFactoryQos_INITIALIZER;
        qos.profile.ignore_resource_profile = DDS_BOOLEAN_TRUE;
        qos.profile.ignore_environment_profile = DDS_BOOLEAN_TRUE;
        qos.profile.ignore_user_profile = DDS_BOOLEAN_TRUE;
        qos.monitoring.enable = DDS_BOOLEAN_FALSE;
        REQUIRE(DDS_DomainParticipantFactory_set_qos(
                DDS_DomainParticipantFactory_get_instance(), &qos) == DDS_RETCODE_OK);
        DDS_DomainParticipantFactoryQos_finalize(&qos);
    }
    if (strncmp(argv[1], "kafka-", 6) == 0) kafka_io(argv[1], argv[2], argv[4], atoi(argv[5]));
    else dds_io(argv[1], atoi(argv[3]), argv[4], atoi(argv[5]));
    return 0;
}
