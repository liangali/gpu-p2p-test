cmake_minimum_required(VERSION 3.0)

project(add)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")
include_directories("/usr/include/level_zero")

add_executable(add add.cpp)
target_link_libraries(add ze_loader)
