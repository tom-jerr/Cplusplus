set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/test)
# add_executable(THREAD ThreadCacheTest.cpp ${CMAKE_SOURCE_DIR}/src/ThreadCache.cpp)
# target_include_directories(THREAD PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(Test Test.cpp)
target_include_directories(Test PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Test PRIVATE mempool)
