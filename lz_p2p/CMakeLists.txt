add_executable(lzp2p lz_p2p.cpp)

include_directories(${CMAKE_SOURCE_DIR}/common)

target_link_libraries(lzp2p commonlib)

target_link_libraries(lzp2p ze_loader)
