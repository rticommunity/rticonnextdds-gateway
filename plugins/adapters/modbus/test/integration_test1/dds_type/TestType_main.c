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

#include <stdio.h>
#include <stdlib.h>
#include "ndds/ndds_c.h"
#include "TestType.h"
#include "TestTypeSupport.h"

/* Define test values */
#define STRING_CONSTANT_VALUE "value"
#define UNSIGNED_CONSTANT_VALUE 11
#define SIGNED_CONSTANT_VALUE -42
#define FLOAT_CONSTANT_VALUE 0.5
#define BOOLEAN_CONSTANT_VALUE 1
#define ENUM_VALUE SOLID_FILL
#define SIGNED_VALUE -1
#define UNSIGNED_VALUE 1
#define VALUE_WITH_OFFSET_DATA_FACTOR(value) 5 + (value) * 10
#define BOOLEAN_VALUE DDS_BOOLEAN_TRUE
#define FLOAT_VALUE 1.5f
#define FLOAT_NEGATIVE_VALUE -1.5f
#define FLOAT_INCREMENT 0.5f
#define SIGNED_ARRAY_VALUE(i) (i) * (-1)
#define UNSIGNED_ARRAY_VALUE(i) (i)
#define BOOLEAN_ARRAY_VALUE(i) i % 2
#define FLOAT_ARRAY_VALUE(i) (i) + FLOAT_INCREMENT
#define FLOAT_ARRAY_NEGATIVE_VALUE(i) (i) * (-1) - FLOAT_INCREMENT


#define ASSERT_COND(cond) \
    do { \
        if (!(cond)) { \
            printf("error assertion: <%s> in line <%d>\n", #cond, __LINE__); \
            goto done; \
        } \
    } while (0)


DDS_Boolean check_received_parameters(TestType_sub *sample)
{
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    int i = 0;

    /* Check holding registers from the writer */

    ASSERT_COND(*(sample->test_type.enum_field) == ENUM_VALUE);
    /*
     * The optional is not set, but the adapter reads a value from
     * the modbus server. This case is the default 0
     */
    ASSERT_COND(*(sample->test_type.optional_non_set) == 0);
    ASSERT_COND(sample->test_type.bool_field == BOOLEAN_VALUE);
    /*
     * The following conversion to DDS_Octet is done to ensure compatibility
     * between 6.0.1, where int8 are represented as octets, and 6.1.0, where
     * int8 are represented by signed int8
     */
    ASSERT_COND((DDS_Octet)sample->test_type.int8_field == (DDS_Octet)SIGNED_VALUE);
    ASSERT_COND(sample->test_type.uint8_field == (DDS_Octet)UNSIGNED_VALUE);
    ASSERT_COND(*(sample->test_type.int16_field) == SIGNED_VALUE);
    ASSERT_COND(sample->test_type.uint16_field == UNSIGNED_VALUE);
    ASSERT_COND(sample->test_type.int32_field == SIGNED_VALUE);
    ASSERT_COND(sample->test_type.uint32_field == UNSIGNED_VALUE);
    ASSERT_COND(sample->test_type.int64_field == SIGNED_VALUE);
    ASSERT_COND(sample->test_type.uint64_field == UNSIGNED_VALUE);
    ASSERT_COND(sample->test_type.float_abcd_field == FLOAT_VALUE);
    ASSERT_COND(sample->test_type.float_badc_field == FLOAT_NEGATIVE_VALUE);
    ASSERT_COND(sample->test_type.float_cdab_field == FLOAT_VALUE);
    ASSERT_COND(sample->test_type.float_dcba_field == FLOAT_NEGATIVE_VALUE);

    for (i = 0; i < 2; ++i) {
        ASSERT_COND(sample->test_type.bool_array_field[i] == i % 2);
        ASSERT_COND(DDS_BooleanSeq_get(
                &sample->test_type.bool_seq_field, i) == (DDS_Boolean)BOOLEAN_ARRAY_VALUE(i));
        /*
         * The following conversion to DDS_Octet is done to ensure compatibility
         * between 6.0.1, where int8 are represented as octets, and 6.1.0, where
         * int8 are represented by signed int8
         */
        ASSERT_COND((DDS_Octet) (sample->test_type.int8_array_field[i])
                == (DDS_Octet) (SIGNED_ARRAY_VALUE(i)));
        ASSERT_COND(DDS_UInt8Seq_get(
                &sample->test_type.uint8_seq_field, i) == (DDS_Octet)UNSIGNED_ARRAY_VALUE(i));
        ASSERT_COND(sample->test_type.int16_array_field[i] == SIGNED_ARRAY_VALUE(i));
        ASSERT_COND(DDS_UnsignedShortSeq_get(
                &sample->test_type.uint16_seq_field, i) == UNSIGNED_ARRAY_VALUE(i));
        ASSERT_COND(sample->test_type.int32_array_field[i] == SIGNED_ARRAY_VALUE(i));
        ASSERT_COND(DDS_UnsignedLongSeq_get(
                &sample->test_type.uint32_seq_field, i) == UNSIGNED_ARRAY_VALUE(i));
        ASSERT_COND(sample->test_type.int64_array_field[i] == SIGNED_ARRAY_VALUE(i));
        ASSERT_COND(DDS_UnsignedLongLongSeq_get(
                &sample->test_type.uint64_seq_field, i) == UNSIGNED_ARRAY_VALUE(i));
        ASSERT_COND(sample->test_type.float_abcd_array_field[i] == FLOAT_ARRAY_VALUE(i));
        ASSERT_COND(DDS_FloatSeq_get(&sample->test_type.float_badc_seq_field, i) ==
                FLOAT_ARRAY_VALUE(i));
        ASSERT_COND(
                sample->test_type.float_cdab_array_field[i]
                        == FLOAT_ARRAY_NEGATIVE_VALUE(i));
        ASSERT_COND(DDS_FloatSeq_get(
                &sample->test_type.float_dcba_seq_field, i)
                        == FLOAT_ARRAY_NEGATIVE_VALUE(i));
    }

    /* Check input registers */
    ASSERT_COND(strcmp(sample->string_constant, STRING_CONSTANT_VALUE) == 0);
    ASSERT_COND(sample->int8_constant == SIGNED_CONSTANT_VALUE);
    ASSERT_COND(sample->int16_constant == SIGNED_CONSTANT_VALUE);
    ASSERT_COND(sample->int32_constant == SIGNED_CONSTANT_VALUE);
    ASSERT_COND(sample->int64_constant == SIGNED_CONSTANT_VALUE);
    ASSERT_COND(sample->uint8_constant == UNSIGNED_CONSTANT_VALUE);
    ASSERT_COND(sample->uint16_constant == UNSIGNED_CONSTANT_VALUE);
    ASSERT_COND(sample->uint32_constant == UNSIGNED_CONSTANT_VALUE);
    ASSERT_COND(sample->uint64_constant == UNSIGNED_CONSTANT_VALUE);
    ASSERT_COND(sample->float32_constant == FLOAT_CONSTANT_VALUE);
    ASSERT_COND(sample->float64_constant == FLOAT_CONSTANT_VALUE);
    ASSERT_COND(sample->boolean_constant == BOOLEAN_CONSTANT_VALUE);
    ASSERT_COND(*sample->input_enum_field == ENUM_VALUE);
    ASSERT_COND(*sample->input_bool_field == DDS_BOOLEAN_TRUE);
    ASSERT_COND(*sample->input_int8_field == UNSIGNED_VALUE);
    ASSERT_COND(*sample->input_uint8_field
            == VALUE_WITH_OFFSET_DATA_FACTOR(UNSIGNED_VALUE));
    ASSERT_COND(*sample->input_int16_field == UNSIGNED_VALUE);
    ASSERT_COND(*sample->input_uint16_field == UNSIGNED_VALUE);
    ASSERT_COND(*sample->input_int32_field == UNSIGNED_VALUE);
    ASSERT_COND(*sample->input_uint32_field == UNSIGNED_VALUE);
    ASSERT_COND(*sample->input_int64_field == UNSIGNED_VALUE);
    ASSERT_COND(*sample->input_uint64_field == UNSIGNED_VALUE);
    ASSERT_COND(*sample->input_float_abcd_field == FLOAT_VALUE);
    ASSERT_COND(*sample->input_float_badc_field == FLOAT_VALUE);
    ASSERT_COND(sample->input_float_cdab_field == FLOAT_VALUE);
    ASSERT_COND(sample->input_float_dcba_field == FLOAT_VALUE);

    for (i = 0; i < 2; ++i) {
        int value = i + 2;
        ASSERT_COND(sample->input_bool_array_field[i] == value % 2);
        ASSERT_COND(DDS_BooleanSeq_get(
                &sample->input_bool_seq_field, i) == BOOLEAN_ARRAY_VALUE(value));
        ASSERT_COND(sample->input_int8_array_field[i] == UNSIGNED_ARRAY_VALUE(value));
        ASSERT_COND(DDS_UInt8Seq_get(
                &sample->input_uint8_seq_field, i)
                        == VALUE_WITH_OFFSET_DATA_FACTOR(UNSIGNED_ARRAY_VALUE(value)));
        ASSERT_COND(sample->input_int16_array_field[i] == UNSIGNED_ARRAY_VALUE(value));
        ASSERT_COND(DDS_UnsignedShortSeq_get(
                &sample->input_uint16_seq_field, i) == UNSIGNED_ARRAY_VALUE(value));
        ASSERT_COND(sample->input_int32_array_field[i] == UNSIGNED_ARRAY_VALUE(value));
        ASSERT_COND(DDS_UnsignedLongSeq_get(
                &sample->input_uint32_seq_field, i) == UNSIGNED_ARRAY_VALUE(value));
        ASSERT_COND(sample->input_int64_array_field[i] == UNSIGNED_ARRAY_VALUE(value));
        ASSERT_COND(DDS_UnsignedLongLongSeq_get(
                &sample->input_uint64_seq_field, i) == UNSIGNED_ARRAY_VALUE(value));
        ASSERT_COND(sample->input_float_abcd_array_field[i] == FLOAT_ARRAY_VALUE(value));
        ASSERT_COND(DDS_FloatSeq_get(&sample->input_float_badc_seq_field, i) ==
                FLOAT_ARRAY_VALUE(value));
        ASSERT_COND(
                sample->input_float_cdab_array_field[i] == FLOAT_ARRAY_VALUE(value));
        ASSERT_COND(DDS_FloatSeq_get(
                &sample->input_float_dcba_seq_field, i)
                        == FLOAT_ARRAY_VALUE(value));
    }

    printf("PASSED TEST OK\n");

    ok = DDS_BOOLEAN_TRUE;
done:
    return ok;
}

/* Delete all entities */
static int tester_shutdown(DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = DDS_DomainParticipant_delete_contained_entities(participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDS_DomainParticipantFactory_delete_participant(
            DDS_TheParticipantFactory, participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Data Distribution Service provides the finalize_instance() method on
    domain participant factory for users who want to release memory used
    by the participant factory. Uncomment the following block of code for
    clean destruction of the singleton. */
    /*
    retcode = DDS_DomainParticipantFactory_finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "finalize_instance error %d\n", retcode);
        status = -1;
    }
    */

    return status;
}

int publish_data_test(DDS_DomainParticipant *participant)
{
    DDS_Publisher *publisher = NULL;
    DDS_Topic *topic_pub = NULL;
    DDS_DataWriter *writer = NULL;
    TestType_pubDataWriter *TestType_pub_writer = NULL;
    TestType_pub *instance = NULL;
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    const char *type_name_pub = NULL;
    int i = 0;
    DDS_WaitSet *waitset_pub = NULL;
    DDS_StatusCondition *status_condition_pub = NULL;
    struct DDS_Duration_t timeout = { 4, 0 };
    struct DDS_ConditionSeq active_conditions = DDS_SEQUENCE_INITIALIZER;
    int errorCode = 0; /* 0 --> OK, -1 --> error */

    /* To customize publisher QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    publisher = DDS_DomainParticipant_create_publisher(
        participant, &DDS_PUBLISHER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        fprintf(stderr, "create_publisher error\n");
        errorCode = -1;
        goto done;
    }

    /* Register type before creating topic */
    type_name_pub = TestType_pubTypeSupport_get_type_name();
    retcode = TestType_pubTypeSupport_register_type(
        participant, type_name_pub);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "register_type error %d\n", retcode);
        errorCode = -1;
        goto done;
    }

    /* To customize topic QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    topic_pub = DDS_DomainParticipant_create_topic(
        participant, "Modbus Adapter Test Pub",
        type_name_pub, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic_pub == NULL) {
        fprintf(stderr, "create_topic error\n");
        errorCode = -1;
        goto done;
    }

    /* To customize data writer QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    writer = DDS_Publisher_create_datawriter(
        publisher, topic_pub,
        &DDS_DATAWRITER_QOS_DEFAULT, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
        fprintf(stderr, "create_datawriter error\n");
        errorCode = -1;
        goto done;
    }
    TestType_pub_writer = TestType_pubDataWriter_narrow(writer);
    if (TestType_pub_writer == NULL) {
        fprintf(stderr, "DataWriter narrow error\n");
        errorCode = -1;
        goto done;
    }

    /* Create data sample for writing */
    instance = TestType_pubTypeSupport_create_data_ex(DDS_BOOLEAN_TRUE);
    if (instance == NULL) {
        fprintf(stderr, "TestType_subTypeSupport_create_data error\n");
        errorCode = -1;
        goto done;
    }

    /* Fill out the instance */
    instance->enum_field = (ShapeFillKind*) malloc(sizeof(ShapeFillKind));
    *instance->enum_field = ENUM_VALUE;
    instance->optional_non_set = NULL;
    instance->bool_field = BOOLEAN_VALUE;
    instance->int8_field = SIGNED_VALUE;
    instance->uint8_field = UNSIGNED_VALUE;
    instance->int16_field = (short*) malloc(sizeof(short));
    *instance->int16_field = SIGNED_VALUE;
    instance->uint16_field = UNSIGNED_VALUE;
    instance->int32_field = SIGNED_VALUE;
    instance->uint32_field = UNSIGNED_VALUE;
    instance->int64_field = SIGNED_VALUE;
    instance->uint64_field = UNSIGNED_VALUE;
    instance->float_abcd_field = FLOAT_VALUE;
    instance->float_badc_field = FLOAT_NEGATIVE_VALUE;
    instance->float_cdab_field = FLOAT_VALUE;
    instance->float_dcba_field = FLOAT_NEGATIVE_VALUE;

    for (i = 0; i < 2; ++i) {
        instance->bool_array_field[i] = i % 2;
        DDS_BooleanSeq_set_length(&instance->bool_seq_field, 2);
        *DDS_BooleanSeq_get_reference(&instance->bool_seq_field, i) =
                BOOLEAN_ARRAY_VALUE(i);
        instance->int8_array_field[i] = SIGNED_ARRAY_VALUE(i);
        DDS_UInt8Seq_set_length(&instance->uint8_seq_field, 2);
        *DDS_UInt8Seq_get_reference(&instance->uint8_seq_field, i) =
                UNSIGNED_ARRAY_VALUE(i);
        instance->int16_array_field[i] = SIGNED_ARRAY_VALUE(i);
        DDS_UnsignedShortSeq_set_length(&instance->uint16_seq_field, 2);
        *DDS_UnsignedShortSeq_get_reference(&instance->uint16_seq_field, i) =
                UNSIGNED_ARRAY_VALUE(i);
        instance->int32_array_field[i] = SIGNED_ARRAY_VALUE(i);
        DDS_UnsignedLongSeq_set_length(&instance->uint32_seq_field, 2);
        *DDS_UnsignedLongSeq_get_reference(&instance->uint32_seq_field, i) =
                UNSIGNED_ARRAY_VALUE(i);
        instance->int64_array_field[i] = SIGNED_ARRAY_VALUE(i);
        DDS_UnsignedLongLongSeq_set_length(&instance->uint64_seq_field, 2);
        *DDS_UnsignedLongLongSeq_get_reference(&instance->uint64_seq_field, i) =
                UNSIGNED_ARRAY_VALUE(i);
        instance->float_abcd_array_field[i] = FLOAT_ARRAY_VALUE(i);
        DDS_FloatSeq_set_length(&instance->float_badc_seq_field, 2);
        *DDS_FloatSeq_get_reference(&instance->float_badc_seq_field, i) =
                FLOAT_ARRAY_VALUE(i);
        instance->float_cdab_array_field[i] = FLOAT_ARRAY_NEGATIVE_VALUE(i);
        DDS_FloatSeq_set_length(&instance->float_dcba_seq_field, 2);
        *DDS_FloatSeq_get_reference(&instance->float_dcba_seq_field, i) =
                FLOAT_ARRAY_NEGATIVE_VALUE(i);
    }

    /*
     * Create a Waitset for the writer with status condition
     * on_subscription_matched
     */
    status_condition_pub = DDS_Entity_get_statuscondition((DDS_Entity *) writer);
    if (status_condition_pub == NULL) {
        printf("get_statuscondition error\n");
        errorCode = -1;
        goto done;
    }

    retcode = DDS_StatusCondition_set_enabled_statuses(
            status_condition_pub,
            DDS_PUBLICATION_MATCHED_STATUS);
    if (retcode != DDS_RETCODE_OK) {
        printf("set_enabled_statuses error\n");
        errorCode = -1;
        goto done;
    }

    waitset_pub = DDS_WaitSet_new();
    if (waitset_pub == NULL) {
        printf("create waitset error\n");
        errorCode = -1;
        goto done;
    }

    retcode = DDS_WaitSet_attach_condition(
            waitset_pub,
            (DDS_Condition *) status_condition_pub);
    if (retcode != DDS_RETCODE_OK) {
        printf("attach_condition error\n");
        errorCode = -1;
        goto done;
    }

    retcode = DDS_WaitSet_wait(waitset_pub, &active_conditions, &timeout);
    if (retcode == DDS_RETCODE_TIMEOUT) {
        printf("Non-matched publication\n");
        errorCode = -1;
        goto done;
    } else if (retcode != DDS_RETCODE_OK) {
        printf("wait returned error: %d\n", retcode);
        errorCode = -1;
        goto done;
    }

    for (i = 0; i < DDS_ConditionSeq_get_length(&active_conditions); ++i) {
        /* Compare with Status Conditions */
        if (DDS_ConditionSeq_get(&active_conditions, i) ==
                (DDS_Condition *) status_condition_pub) {
            /* A status condition triggered--see which ones */
            DDS_StatusMask triggeredmask;
            triggeredmask = DDS_Entity_get_status_changes(
                    (DDS_Entity *) TestType_pub_writer);

            /* Subscription matched */
            if (triggeredmask & DDS_PUBLICATION_MATCHED_STATUS) {
                retcode = TestType_pubDataWriter_write(
                        TestType_pub_writer, instance, &instance_handle);
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr, "write error %d\n", retcode);
                    errorCode = -1;
                    goto done;
                }
            } else {
                    fprintf(stderr, "error: non matched subscription\n");
                    errorCode = -1;
                    goto done;
            }
        }
    }

done:
    DDS_ConditionSeq_finalize(&active_conditions);
    return errorCode;
}

int read_data_test(DDS_DomainParticipant *participant)
{
    DDS_Subscriber *subscriber = NULL;
    DDS_Topic *topic_sub = NULL;
    DDS_DataReader *reader = NULL;
    DDS_ReturnCode_t retcode;
    const char *type_name_sub = NULL;
    int i = 0;
    DDS_WaitSet *waitset_sub = NULL;
    DDS_StatusCondition *status_condition_sub = NULL;
    struct DDS_Duration_t timeout = { 4, 0 };
    struct DDS_ConditionSeq active_conditions = DDS_SEQUENCE_INITIALIZER;
    TestType_subDataReader *TestType_sub_reader = NULL;
    int errCode = 0;

    /*************************** Reader side **********************/
    /* To customize subscriber QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    subscriber = DDS_DomainParticipant_create_subscriber(
            participant, &DDS_SUBSCRIBER_QOS_DEFAULT, NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        fprintf(stderr, "create_subscriber error\n");
        errCode = -1;
        goto done;
    }

    /* Register the type before creating the topic */
    type_name_sub = TestType_subTypeSupport_get_type_name();
    retcode = TestType_subTypeSupport_register_type(participant, type_name_sub);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "register_type error %d\n", retcode);
        errCode = -1;
        goto done;
    }

    /* To customize topic QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    topic_sub = DDS_DomainParticipant_create_topic(
        participant, "Modbus Adapter Test Sub",
        type_name_sub, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic_sub == NULL) {
        fprintf(stderr, "create_topic error\n");
        errCode = -1;
        goto done;
    }

    /* To customize data reader QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    reader = DDS_Subscriber_create_datareader(
        subscriber, DDS_Topic_as_topicdescription(topic_sub),
        &DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
    if (reader == NULL) {
        fprintf(stderr, "create_datareader error\n");
        errCode = -1;
        goto done;
    }

    // Get narrowed datareader
    TestType_sub_reader = TestType_subDataReader_narrow(reader);
    if (TestType_sub_reader == NULL) {
        printf("DataReader narrow error\n");
        errCode = -1;
        goto done;
    }

    status_condition_sub = DDS_Entity_get_statuscondition((DDS_Entity *) reader);
    if (status_condition_sub == NULL) {
        printf("get_statuscondition error\n");
        errCode = -1;
        goto done;
    }

    retcode = DDS_StatusCondition_set_enabled_statuses(
            status_condition_sub,
            DDS_DATA_AVAILABLE_STATUS);
    if (retcode != DDS_RETCODE_OK) {
        printf("set_enabled_statuses error\n");
        errCode = -1;
        goto done;
    }

    waitset_sub = DDS_WaitSet_new();
    if (waitset_sub == NULL) {
        printf("create waitset error\n");
        errCode = -1;
        goto done;
    }


    retcode = DDS_WaitSet_attach_condition(
            waitset_sub,
            (DDS_Condition *) status_condition_sub);
    if (retcode != DDS_RETCODE_OK) {
        printf("attach_condition error\n");
        errCode = -1;
        goto done;
    }

    retcode = DDS_WaitSet_wait(waitset_sub, &active_conditions, &timeout);
    if (retcode == DDS_RETCODE_TIMEOUT) {
        printf("reader wait timed out\n");
        errCode = -1;
        goto done;
    } else if (retcode != DDS_RETCODE_OK) {
        printf("wait returned error: %d\n", retcode);
        errCode = -1;
        goto done;
    }

    for (i = 0; i < DDS_ConditionSeq_get_length(&active_conditions); ++i) {
        /* Compare with Status Conditions */
        if (DDS_ConditionSeq_get(&active_conditions, i) ==
                (DDS_Condition *) status_condition_sub) {
            /* A status condition triggered--see which ones */
            DDS_StatusMask triggeredmask;
            triggeredmask = DDS_Entity_get_status_changes(
                    (DDS_Entity *) TestType_sub_reader);

            /* when receive data matched */
            if (triggeredmask & DDS_DATA_AVAILABLE_STATUS) {
                struct TestType_subSeq data_seq =
                        DDS_SEQUENCE_INITIALIZER;
                struct DDS_SampleInfoSeq info_seq =
                        DDS_SEQUENCE_INITIALIZER;
                TestType_sub *sample = NULL;

                retcode = TestType_subDataReader_take(
                        TestType_sub_reader,
                        &data_seq,
                        &info_seq,
                        1, /* read only one sample */
                        DDS_ANY_SAMPLE_STATE,
                        DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);
                if (retcode != DDS_RETCODE_OK) {
                    TestType_subSeq_finalize(&data_seq);
                    DDS_SampleInfoSeq_finalize(&info_seq);
                    printf("Error taking samples\n");
                    goto done;
                }
                sample = TestType_subSeq_get_reference(&data_seq, 0);

                check_received_parameters(sample);

                TestType_subDataReader_return_loan(
                        TestType_sub_reader,
                        &data_seq,
                        &info_seq);
            }
        }
    }

done:
    DDS_ConditionSeq_finalize(&active_conditions);
    return errCode;
}

int main(int argc, char *argv[])
{
    int domain_id = 0;
    int sample_count = 0; /* infinite loop */
    DDS_DomainParticipant *participant = NULL;
    struct DDS_Duration_t timeout = { 2, 0 };

    if (argc >= 2) {
        domain_id = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /* To customize participant QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    participant = DDS_DomainParticipantFactory_create_participant(
        DDS_TheParticipantFactory, domain_id, &DDS_PARTICIPANT_QOS_DEFAULT,
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        fprintf(stderr, "create_participant error\n");
        tester_shutdown(participant);
        return -1;
    }

    if (publish_data_test(participant) != 0) {
        fprintf(stderr, "publish_data_test error\n");
        tester_shutdown(participant);
        return -1;
    }

    //wait a couple of seconds to allow the adapter to copy the values
    NDDS_Utility_sleep(&timeout);

    if (read_data_test(participant) != 0) {
        fprintf(stderr, "read_data_test error\n");
        tester_shutdown(participant);
        return -1;
    }

    /* Cleanup and delete all entities */
    return tester_shutdown(participant);
}

