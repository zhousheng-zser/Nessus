cmake_minimum_required(VERSION 3.14.3)

include(simdjson-flags.cmake)

add_library(simdjson-include-source INTERFACE)
target_include_directories(simdjson-include-source INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../../../3rd-party/cpp/simdjson/include)
target_include_directories(simdjson-include-source INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../../../3rd-party/cpp/simdjson/src)

add_library(simdjson-source INTERFACE)
target_sources(simdjson-source INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../../../3rd-party/cpp/simdjson/src/simdjson.cpp)
target_link_libraries(simdjson-source INTERFACE simdjson-include-source)

option(SIMDJSON_BUILD_STATIC "Build a static library" ON) # turning it on disables the production of a dynamic library
target_compile_options(simdjson-source INTERFACE -Wno-deprecated-declarations)
if(SIMDJSON_BUILD_STATIC)
  MESSAGE( STATUS "Building a static library." )
  add_library(simdjson STATIC "")
else()
  MESSAGE( STATUS "Building a dynamic library." )
  add_library(simdjson SHARED "")
  target_compile_definitions(simdjson INTERFACE SIMDJSON_USING_LIBRARY=1)
  if(MSVC)
    MESSAGE( STATUS "Building a Windows DLL using Visual Studio, exporting all symbols automatically." )
    set_target_properties(simdjson PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
  endif()
endif()

target_link_libraries(simdjson PRIVATE simdjson-source simdjson-internal-flags)
