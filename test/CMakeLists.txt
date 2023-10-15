add_executable(UNITTEST UnitTest.cpp)
target_link_libraries(UNITTEST neofileop gtest gtest_main )
# ------ set binary directory ------
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)