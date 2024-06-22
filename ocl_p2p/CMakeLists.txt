add_executable(oclp2p main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/common)

target_link_libraries(oclp2p commonlib)

target_link_libraries(oclp2p OpenCL)
