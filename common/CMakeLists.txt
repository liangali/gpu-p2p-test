add_library(commonlib STATIC ocl_context.cpp lz_context.cpp usm_api.cpp)

target_include_directories(commonlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} /usr/include/level_zero)
