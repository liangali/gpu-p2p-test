# cmake_minimum_required(VERSION 3.0)

# project(p2p)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")
# include_directories("/usr/include/level_zero")

# add_executable(p2p p2p.cpp context.cpp)
# target_link_libraries(p2p ze_loader)


add_executable(p2p p2p.cpp)

include_directories(${CMAKE_SOURCE_DIR}/common)

target_link_libraries(p2p commonlib)

target_link_libraries(p2p ze_loader)
