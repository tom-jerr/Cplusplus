find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
enable_testing()

add_executable(PoolOuterSortTest PoolOuterSortTest.cpp)
target_link_libraries(PoolOuterSortTest OuterSort pthread gtest)

add_executable(SafeQueueTest SafeQueueTest.cpp)
target_link_libraries(SafeQueueTest pthread gtest)
