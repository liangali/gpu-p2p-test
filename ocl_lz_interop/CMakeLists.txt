cmake_minimum_required(VERSION 2.8)

project(interop)

link_directories(/usr/lib/x86_64-linux-gnu)
include_directories(/usr/include/level_zero)

add_executable(interop interop.cpp ocl_context.cpp lz_context.cpp usm_api.cpp)
target_link_libraries(interop OpenCL ze_loader)
