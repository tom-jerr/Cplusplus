cmake_minimum_required(VERSION 3.16)

# --- Project settings ---

# ------ Project name ------
project(neolog)

# ------ Standard ------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------ Build type ------
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# ------ include directory ------
include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(third_party/googletest)