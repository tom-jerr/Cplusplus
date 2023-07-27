set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/src)
add_library(mempool STATIC ThreadCache.cpp CentralCache.cpp PageCache.cpp)
target_include_directories(mempool PRIVATE ${CMAKE_SOURCE_DIR}/include)
