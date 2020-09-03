cmake_minimum_required(VERSION 3.14.3)
set(module_name plugin_register)

file(GLOB src ${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister/*.cpp)
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister/demo.cpp")

add_library(${module_name} SHARED ${src})

target_include_directories(${module_name} PRIVATE ${COMMON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_definitions(${module_name} PRIVATE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
	target_link_directories(${module_name} PRIVATE ${COMMON_LIBRARY_DIRS})
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_link_libraries(${module_name} PRIVATE stdc++fs)
endif()

target_link_libraries(${module_name} PRIVATE ${COMMON_LIBRARIES})
