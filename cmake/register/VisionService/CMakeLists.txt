cmake_minimum_required(VERSION 3.14.3)
set(module_name vision_service)
file(GLOB src ${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/VisionService/*.cpp)
file(GLOB header ${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/VisionService/*.hpp)

add_library(${module_name} SHARED ${src} ${header})

target_include_directories(${module_name} PRIVATE 
			${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister 
			${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp 
			${COMMON_INCLUDE_DIRS})

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
	target_link_directories(${module_name} PRIVATE ${COMMON_LIBRARY_DIRS})
endif()

target_link_libraries(${module_name} PRIVATE plugin_register ${COMMON_LIBRARIES})
add_dependencies(${module_name} plugin_register)
