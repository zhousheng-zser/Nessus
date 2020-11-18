cmake_minimum_required(VERSION 3.14.3)
set(module_name parser)
file(GLOB src ${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/jni/*.cpp)
file(GLOB header ${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/*.h)
if(NOT WITH_JNI AND NOT ANDROID_NDK)
	list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/jni/parser_jni.cpp")
endif()

add_library(${module_name} SHARED ${src} ${header})
if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
	target_compile_options(${module_name} PRIVATE -Wno-narrowing)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_definitions(${module_name} PRIVATE -DPARSER_C_EXPORT -DJSON_DLL_BUILD -DNOMINMAX -DNOGDI)
endif()

target_include_directories(${module_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../3rd-party/cpp/libyuv/include 
			${CMAKE_CURRENT_SOURCE_DIR}/../../register/cpp/PluginRegister 
			${CMAKE_CURRENT_SOURCE_DIR}/../../register/cpp/VisionService 
			${CMAKE_CURRENT_SOURCE_DIR}/../../license/cpp/client 
			${COMMON_INCLUDE_DIRS} 
			${JNI_INCLUDE_DIRS})

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
	target_link_directories(${module_name} PRIVATE ${COMMON_LIBRARY_DIRS})
endif()

target_link_libraries(${module_name} yuv plugin_register license_client ${COMMON_LIBRARIES} Threads::Threads)
add_dependencies(${module_name} yuv plugin_register license_client)

if((CMAKE_SIZEOF_VOID_P EQUAL 8) AND USE_SIMDJSON)
target_compile_definitions(${module_name} PRIVATE -DUSE_SIMDJSON)
target_include_directories(${module_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../3rd-party/cpp/simdjson/singleheader)
target_link_libraries(${module_name} simdjson)
add_dependencies(${module_name} simdjson)
endif()
