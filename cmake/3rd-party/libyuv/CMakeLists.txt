cmake_minimum_required(VERSION 3.14.3)
set(module_name yuv)
file(GLOB src ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/libyuv/src/*.cc)
file(GLOB header ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/libyuv/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/libyuv/include/libyuv/*.h)

add_library(${module_name} STATIC ${src} ${header})

target_include_directories(${module_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/libyuv/include)

