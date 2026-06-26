cmake_minimum_required(VERSION 4.1)
project(HyperLogLogCPP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_library(hyperll src/hyperll.cpp)

add_executable(small tests/small.cpp)
target_link_libraries(small hyperll)
add_executable(structs_tests tests/structs.cpp)
