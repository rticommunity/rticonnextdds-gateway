cmake_minimum_required(VERSION 3.10)

# Also allow running these unit tests without configuring the gateway or its
# third-party submodules. Only the Connext SDK is needed; Kafka is a test double.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(KafkaAdapterUnitTests LANGUAGES C)
    enable_testing()
    list(APPEND CMAKE_MODULE_PATH
        "${CONNEXTDDS_DIR}/resource/cmake"
        "$ENV{NDDSHOME}/resource/cmake")
    find_package(RTIConnextDDS REQUIRED COMPONENTS routing_service)
endif()

option(KAFKA_TEST_INTEGRATION "Build local-broker/DDS regression tests" OFF)
if(KAFKA_TEST_INTEGRATION)
    add_subdirectory(integration)
endif()

add_executable(kafka_adapter_unit_tests
    KafkaAdapterTests.c
    KafkaLifecycleTests.c
    FakeKafka.c
    "../srcC/KafkaStreamWriter.c"
    "../srcC/KafkaStreamReader.c")
target_include_directories(kafka_adapter_unit_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/fakes"
    "${CMAKE_CURRENT_SOURCE_DIR}/../srcC")
target_link_libraries(kafka_adapter_unit_tests PRIVATE
    RTIConnextDDS::routing_service_c)
set_target_properties(kafka_adapter_unit_tests PROPERTIES
    C_STANDARD 99 C_STANDARD_REQUIRED YES)

set(kafka_stream_cases
        writer_text writer_binary writer_nonterminated writer_empty
        writer_text_terminated writer_text_nonterminated writer_text_empty
        writer_text_nul_only writer_text_double_nul writer_text_embedded_zero
        writer_text_retry writer_text_batch writer_binary_terminated
        writer_batch writer_queue_full writer_enqueue_error writer_missing_payload
        writer_partial_failure writer_zero_count
        writer_leading_zero writer_trailing_zero writer_large writer_retry_error
        writer_last_failure writer_missing_later writer_repeated writer_mixed_batch
        reader_binary reader_timeout reader_error_recovery reader_empty
        reader_tombstone reader_stopped reader_idle reader_repeated_errors
        reader_repeated_timeouts reader_repeated_messages reader_invalid_sample
        reader_synchronous reader_read_twice
        reader_shutdown_waiting reader_shutdown_polling reader_shutdown_pending_loan)
foreach(case IN LISTS kafka_stream_cases)
    add_test(NAME kafka_${case} COMMAND kafka_adapter_unit_tests ${case})
    set_tests_properties(kafka_${case} PROPERTIES TIMEOUT 10)
endforeach()

set(kafka_lifecycle_cases adapter_callbacks adapter_allocation_failure
    connection_success connection_missing_bootstrap connection_allocation_failure)
foreach(case IN ITEMS success missing_topic missing_name allocation_1
        conf_allocation invalid_bootstrap invalid_property client_failure secret
        format_text format_binary format_invalid format_empty
        undelivered delivery_success delivery_failure)
    list(APPEND kafka_lifecycle_cases create_writer_${case})
endforeach()
foreach(case IN ITEMS success missing_topic allocation_1 allocation_2 allocation_3
        allocation_4 conf_allocation invalid_bootstrap invalid_group invalid_property
        client_failure redirect_failure subscription_allocation subscription_add
        subscribe_failure support_failure sample_failure semaphore_1 semaphore_2 semaphore_3
        initial_give thread_failure group_override secret pending_loan close_error
        join_failure stop_signal_failure stop_wakeup_failure)
    list(APPEND kafka_lifecycle_cases create_reader_${case})
endforeach()
foreach(case IN LISTS kafka_lifecycle_cases)
    add_test(NAME kafka_${case} COMMAND kafka_adapter_unit_tests ${case})
    set_tests_properties(kafka_${case} PROPERTIES TIMEOUT 10)
endforeach()

# RTI's macOS shared libraries use @loader_path install names. Supply the SDK
# library path to CTest when these unit tests share the integration build.
if(APPLE AND BUILD_SHARED_LIBS)
    foreach(case IN LISTS kafka_stream_cases kafka_lifecycle_cases)
        set_tests_properties(kafka_${case} PROPERTIES ENVIRONMENT
            "DYLD_LIBRARY_PATH=${CONNEXTDDS_DIR}/lib/${CONNEXTDDS_ARCH}:$ENV{DYLD_LIBRARY_PATH}")
    endforeach()
endif()

# Reject misspelled case names instead of silently running a default scenario.
set(kafka_case_literals "")
foreach(case IN LISTS kafka_stream_cases kafka_lifecycle_cases)
    string(APPEND kafka_case_literals "    \"${case}\",\n")
endforeach()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/KafkaTestCases.h"
    "static const char *const kafka_test_cases[] = {\n${kafka_case_literals}};\n")
target_include_directories(kafka_adapter_unit_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

option(KAFKA_TEST_SANITIZERS "Instrument Kafka unit tests with address/undefined sanitizers" OFF)
option(KAFKA_TEST_COVERAGE "Instrument Kafka unit tests for gcov coverage" OFF)
if(KAFKA_TEST_SANITIZERS OR KAFKA_TEST_COVERAGE)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "Kafka test instrumentation requires Clang or GCC")
    endif()
    if(KAFKA_TEST_SANITIZERS)
        target_compile_options(kafka_adapter_unit_tests PRIVATE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_libraries(kafka_adapter_unit_tests PRIVATE -fsanitize=address,undefined)
    endif()
    if(KAFKA_TEST_COVERAGE)
        target_compile_options(kafka_adapter_unit_tests PRIVATE --coverage)
        target_link_libraries(kafka_adapter_unit_tests PRIVATE --coverage)
    endif()
endif()
