add_executable(memtest main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/common)

target_link_libraries(memtest commonlib)

target_link_libraries(memtest OpenCL)
