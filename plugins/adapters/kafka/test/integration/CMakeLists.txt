get_filename_component(kafka_sources "${CMAKE_CURRENT_SOURCE_DIR}/../../srcC" ABSOLUTE)
get_filename_component(gateway_root "${kafka_sources}/../../../.." ABSOLUTE)
set(KAFKA_RDKAFKA_ROOT "" CACHE PATH "Installed librdkafka prefix")
set(KAFKA_TEST_BASELINE_REF "d8250299084b2150fa1b53103b1f0d020a3560e7" CACHE STRING "Git revision for before/after comparison")
set(KAFKA_TEST_BOOTSTRAP "127.0.0.1:19092" CACHE STRING "Integration broker address")
set(KAFKA_TEST_ROUTING_SERVICE
    "${CONNEXTDDS_DIR}/resource/app/bin/${CONNEXTDDS_ARCH}/rtiroutingserviceapp"
    CACHE FILEPATH "Routing Service executable for integration tests")
if(NOT BUILD_SHARED_LIBS)
    message(FATAL_ERROR "Use -DBUILD_SHARED_LIBS=ON so loaded plugins share the Routing Service DDS runtime")
endif()

# Deliberately exclude test/fakes: this target uses the real Kafka ABI.
find_path(KAFKA_REAL_INCLUDE_DIR rdkafka.h HINTS
    "${KAFKA_RDKAFKA_ROOT}/include/librdkafka"
    "${gateway_root}/third-party/librdkafka/src")
find_library(KAFKA_REAL_LIBRARY NAMES rdkafka HINTS "${KAFKA_RDKAFKA_ROOT}/lib")
if(NOT KAFKA_REAL_INCLUDE_DIR OR NOT KAFKA_REAL_LIBRARY)
    message(FATAL_ERROR "Install librdkafka and set KAFKA_RDKAFKA_ROOT for integration tests")
endif()

set(kafka_source_files KafkaAdapter.c KafkaConnection.c KafkaStreamReader.c KafkaStreamWriter.c)
function(kafka_test_plugin target source_dir)
    set(sources "")
    foreach(file IN LISTS kafka_source_files)
        list(APPEND sources "${source_dir}/${file}")
    endforeach()
    add_library(${target} SHARED ${sources})
    target_include_directories(${target} PRIVATE "${source_dir}" "${KAFKA_REAL_INCLUDE_DIR}")
    target_link_libraries(${target} PRIVATE RTIConnextDDS::routing_service_c "${KAFKA_REAL_LIBRARY}")
endfunction()

kafka_test_plugin(kafka_integration_modified "${kafka_sources}")
if(KAFKA_TEST_BASELINE_REF)
    find_package(Git REQUIRED)
    set(baseline_sources "${CMAKE_CURRENT_BINARY_DIR}/baseline")
    file(MAKE_DIRECTORY "${baseline_sources}")
    foreach(file IN ITEMS ${kafka_source_files}
            KafkaAdapter.h KafkaConnection.h KafkaStreamReader.h KafkaStreamWriter.h)
        execute_process(COMMAND "${GIT_EXECUTABLE}" show
            "${KAFKA_TEST_BASELINE_REF}:plugins/adapters/kafka/srcC/${file}"
            WORKING_DIRECTORY "${gateway_root}"
            OUTPUT_FILE "${baseline_sources}/${file}"
            RESULT_VARIABLE result ERROR_VARIABLE error)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Cannot snapshot Kafka baseline: ${error}")
        endif()
    endforeach()
    kafka_test_plugin(kafka_integration_baseline "${baseline_sources}")
endif()

set(common_transformation "${gateway_root}/common/transformation/srcC")
set(common_dds "${gateway_root}/common/dds_specific/srcC")
add_library(kafka_integration_json SHARED
    "${gateway_root}/plugins/transformations/json/srcC/JsonTransformation.c"
    "${common_transformation}/TransformationInfrastructure.c"
    "${common_transformation}/Transformation.c"
    "${common_transformation}/TransformationPlugin.c"
    "${common_transformation}/TransformationUserPlugin.c"
    "${common_dds}/SequenceHelpers.c"
    "${common_dds}/DynamicDataHelpers.c")
target_include_directories(kafka_integration_json PRIVATE
    "${common_transformation}" "${common_dds}"
    "${gateway_root}/plugins/transformations/json/srcC")
target_link_libraries(kafka_integration_json PRIVATE RTIConnextDDS::routing_service_c)

add_executable(kafka_integration_client IntegrationClient.c)
target_include_directories(kafka_integration_client PRIVATE "${KAFKA_REAL_INCLUDE_DIR}")
target_link_libraries(kafka_integration_client PRIVATE
    RTIConnextDDS::routing_service_c "${KAFKA_REAL_LIBRARY}")
set_target_properties(kafka_integration_client PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED YES)
add_executable(kafka_integration_router RoutingServiceHost.c)
target_link_libraries(kafka_integration_router PRIVATE RTIConnextDDS::routing_service_c)

find_program(KAFKA_TEST_PYTHON NAMES python3)
if(NOT KAFKA_TEST_PYTHON)
    message(FATAL_ERROR "Python 3 is required for integration tests")
endif()
set(baseline_arguments "")
if(TARGET kafka_integration_baseline)
    list(APPEND baseline_arguments --baseline "$<TARGET_FILE:kafka_integration_baseline>")
endif()
add_test(NAME kafka_broker_regression
    COMMAND "${KAFKA_TEST_PYTHON}" "${CMAKE_CURRENT_SOURCE_DIR}/run_integration.py"
        --client "$<TARGET_FILE:kafka_integration_client>"
        --modified "$<TARGET_FILE:kafka_integration_modified>"
        ${baseline_arguments}
        --json-plugin "$<TARGET_FILE:kafka_integration_json>"
        --routing-service "${KAFKA_TEST_ROUTING_SERVICE}"
        --sdk-dir "${CONNEXTDDS_DIR}" --architecture "${CONNEXTDDS_ARCH}"
        --bootstrap "${KAFKA_TEST_BOOTSTRAP}"
        --artifacts "${CMAKE_CURRENT_BINARY_DIR}/results")
set_tests_properties(kafka_broker_regression PROPERTIES TIMEOUT 360 LABELS "kafka;integration")
