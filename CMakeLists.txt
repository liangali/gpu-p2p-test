cmake_minimum_required(VERSION 3.10)

project(MyProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(interop)
add_subdirectory(common)

include_directories(/usr/include/level_zero)
link_directories(/usr/lib/x86_64-linux-gnu/)
