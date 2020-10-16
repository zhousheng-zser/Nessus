cmake_minimum_required(VERSION 3.14.3)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	message(WARNING "CMAKE_TOOLCHAIN_FILE: please specify cmkae toolchain file in cmake command line.")
endif()
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

# For Visual Studio Generator (not Ninja), it is unneccessary to set CMAKE_MAKE_PROGRAM.
if (NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio")
	if(NOT DEFINED CMAKE_MAKE_PROGRAM)
		message(WARNING "CMAKE_MAKE_PROGRAM: please specify make program in cmake command line.")
	endif()

	message(STATUS "CMAKE_MAKE_PROGRAM: ${CMAKE_MAKE_PROGRAM}")
endif()

project(Nessus C CXX)
option(WITH_JNI "Compile java native interface." OFF)
option(CONTAINS_LICENSE_SERVER "Compile the license server." OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

if(WITH_JNI)
	if((NOT DEFINED ANDROID_NDK_TOOLCHAIN_INCLUDED) OR (ANDROID_NDK_TOOLCHAIN_INCLUDED EQUAL FALSE))
		if(NOT DEFINED JNI_INCLUDE_DIRS)
			message(FATAL_ERROR "JNI_INCLUDE_DIRS NOT DEFINED: You must specify jni header directories.")
		endif()
	endif()
endif()

set(COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../common/include ${CMAKE_CURRENT_SOURCE_DIR}/../common/include/Primitives)

if(NOT DEFINED COMMON_LIBRARY_DIRS)
	message(FATAL_ERROR "COMMON_LIBRARY_DIRS NOT DEFINED: You must specify common library's directories.")
endif()

list(APPEND COMMON_LIBRARIES_NAMES primitives)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	foreach(library_name ${COMMON_LIBRARIES_NAMES})
		find_library(library
			NAMES ${library_name}
			PATHS ${COMMON_LIBRARY_DIRS}
			NO_DEFAULT_PATH
		)
		message(STATUS "${library_name} ${library}")
		list(APPEND COMMON_LIBRARIES ${library})
		unset(library CACHE)
	endforeach()
else()
	set(COMMON_LIBRARIES ${COMMON_LIBRARIES_NAMES})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
	message(STATUS "ANDROID_NDK: ${ANDROID_NDK}")
	message(STATUS "ANDROID_ABI: ${ANDROID_ABI}")
	message(STATUS "ANDROID_PLATFORM: ${ANDROID_PLATFORM}")
	message(STATUS "ANDROID_TOOLCHAIN: ${ANDROID_TOOLCHAIN}")
	message(STATUS "ANDROID_STL: ${ANDROID_STL}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
	add_compile_options(-frtti -fexceptions)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
	add_compile_options(-fPIC -Wall -fpermissive)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_IMPORT_LIBRARY_PREFIX lib)
	set(CMAKE_SHARED_LIBRARY_PREFIX lib)
	set(CMAKE_STATIC_LIBRARY_PREFIX lib)
endif()

include(Dependencies.cmake)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Consuming libfmt.
add_definitions(-DFMT_SHARED)

add_subdirectory(3rd-party/simdjson)
add_subdirectory(3rd-party/libyuv)
add_subdirectory(3rd-party/openssl)
add_subdirectory(register/VisionService)
add_subdirectory(register/PluginRegister)
add_subdirectory(parser)
add_subdirectory(license)
add_subdirectory(license/client)

if(CONTAINS_LICENSE_SERVER)
	set(PostgreSQL_ROOT "E:/Libs/pgsql-11.5-1")
	set(PostgreSQL_LIBRARY ${PostgreSQL_ROOT}/lib/libpq.lib)
	add_subdirectory(3rd-party/libpqxx)
	add_subdirectory(license/server)
endif()
