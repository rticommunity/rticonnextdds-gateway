include(ConnextDdsCodegen)

connextdds_rtiddsgen_run(
	IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/types.idl"
	LANG "C++11"
	OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated"
	EXTRA_ARGS -unboundedSupport
)

add_library(unbound2bound_example_types STATIC
	"${CMAKE_CURRENT_BINARY_DIR}/generated/types.cxx"
	"${CMAKE_CURRENT_BINARY_DIR}/generated/typesPlugin.cxx"
)
target_include_directories(unbound2bound_example_types PUBLIC
	"${CMAKE_CURRENT_BINARY_DIR}/generated"
)
target_link_libraries(unbound2bound_example_types PUBLIC RTIConnextDDS::cpp2_api)

add_executable(unbound2bound_publisher publisher.cxx)
target_link_libraries(unbound2bound_publisher PRIVATE unbound2bound_example_types)

add_executable(unbound2bound_subscriber subscriber.cxx)
target_link_libraries(unbound2bound_subscriber PRIVATE unbound2bound_example_types)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/RsUnbounded2Bounded.xml"
	"${CMAKE_CURRENT_BINARY_DIR}/RsUnbounded2Bounded.xml"
	COPYONLY
)

install(TARGETS unbound2bound_publisher unbound2bound_subscriber
	DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}")
install(FILES RsUnbounded2Bounded.xml types.idl README.md
	DESTINATION "${STAGING_EXAMPLES_DIR}/${STAGING_PLUGIN_DIR}")
