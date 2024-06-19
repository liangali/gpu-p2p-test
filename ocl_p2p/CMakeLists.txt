cmake_minimum_required(VERSION 2.8)

project(oclp2p)

# include_directories (oclp2p ${CMAKE_CURRENT_LIST_DIR}/../../compute-runtime/third_party/opencl_headers)

link_directories(/usr/lib/x86_64-linux-gnu)

add_executable(oclp2p main.cpp context.cpp)
target_link_libraries(oclp2p OpenCL)
