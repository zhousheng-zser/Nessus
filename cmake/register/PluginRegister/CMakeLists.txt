add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
file(GLOB plugin_register_source_files ${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister/dllmain.cpp 
					${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister/plugin_interface.cpp 
					${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister/plugin_manager.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister 
			${COMMON_INCLUDE})

add_library(PluginRegister SHARED ${plugin_register_source_files})

