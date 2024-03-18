add_executable(LOGGER_UNITTEST ${CMAKE_CURRENT_SOURCE_DIR}/UnitTest.cpp)
target_link_libraries(LOGGER_UNITTEST LOGGER gtest gmock gtest_main) 
add_executable(LOGGER_BENCHMARK ${CMAKE_CURRENT_SOURCE_DIR}/Benchmark.cpp)
target_link_libraries(LOGGER_BENCHMARK LOGGER gtest gmock gtest_main)