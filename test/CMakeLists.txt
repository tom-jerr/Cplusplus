file(GLOB_RECURSE IOURING_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*test.cpp")

include(GoogleTest)
foreach (iouring_test_source ${IOURING_TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(iouring_test_filename ${iouring_test_source} NAME)
    string(REPLACE ".cpp" "" iouring_test_name ${iouring_test_filename})

    # Add the test target separately and as part of "make check-tests".
    add_executable(${iouring_test_name} ${iouring_test_source})
    # add_backward(${iouring_test_name})
    # add_dependencies(build-tests ${iouring_test_name})
    # add_dependencies(check-tests ${iouring_test_name})

    gtest_discover_tests(${iouring_test_name}
            EXTRA_ARGS
            --gtest_output=xml:${CMAKE_BINARY_DIR}/test/${iouring_test_name}.xml
            --gtest_catch_exceptions=0
            DISCOVERY_TIMEOUT 120
            PROPERTIES
            TIMEOUT 120
            )

    target_link_libraries(${iouring_test_name} uring gtest gmock_main muduo_base muduo_net)

    # Set test target properties and dependencies.
    set_target_properties(${iouring_test_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
            COMMAND ${iouring_test_name}
            )
endforeach ()
