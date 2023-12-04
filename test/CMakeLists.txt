find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
# add_executable(LockFreeQueueTest LockFreeQueueTest.cpp)
# target_link_libraries(LockFreeQueueTest pthread)
# add_executable(ThreadPoolTest ThreadPoolTest.cpp)
# target_link_libraries(ThreadPoolTest pthread)

# # 单线程外部排序测试
# add_executable(SortTest OuterSortTest.cpp)

# add_executable(LogTest LogTest.cpp)

# add_executable(PoolOuterSortTest PoolOuterSortTest.cpp)
# target_link_libraries(PoolOuterSortTest pthread)

add_executable(SafeQueueTest SafeQueueTest.cpp)
target_link_libraries(SafeQueueTest pthread gtest)

# add_executable(SortTest SortTest.cpp)
# target_link_libraries(SortTest pthread gtest)

add_executable(UtilsTest UtilsTest.cpp)
target_link_libraries(UtilsTest pthread gtest CLUtils)