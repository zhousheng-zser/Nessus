cmake_minimum_required(VERSION 3.14.3)
project(Nessus C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
if(UNIX OR APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall")
endif()

set(COMMON_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../common/include 
			${CMAKE_CURRENT_SOURCE_DIR}/../common/include/Primitives)
set(COMMON_LIB_PATH /home/build_slave/ZYF/Excalibur/cmake/build/lib)

set(VISION_LIBRARY_DIRECTORIES /home/build_slave/ZYF/Excalibur/cmake/build/lib)

set(JNI_INCLUDE $ENV{JAVA_HOME}/include $ENV{JAVA_HOME}/include/linux)

add_subdirectory(3rd-party/simdjson)
add_subdirectory(register/VisionService)
add_subdirectory(register/PluginRegister)
add_subdirectory(parser)
