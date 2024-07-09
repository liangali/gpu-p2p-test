cmake_minimum_required(VERSION 3.10)

project(gputest VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")

add_subdirectory(common)
add_subdirectory(lz_p2p)
add_subdirectory(ocl_p2p)
add_subdirectory(interop)
add_subdirectory(memtest)

include_directories(/usr/include/level_zero)
link_directories(/usr/lib/x86_64-linux-gnu/)
