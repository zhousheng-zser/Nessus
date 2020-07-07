file(GLOB parser_source_files ${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/*.cpp 
				${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/jni/*.cpp) 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../3rd-party/cpp/simdjson/include 
			${CMAKE_CURRENT_SOURCE_DIR}/../../register/cpp/PluginRegister 
			${COMMON_INCLUDE} 
			${JNI_INCLUDE})

link_directories(${VISION_LIBRARY_DIRECTORIES})

add_library(Parser SHARED ${parser_source_files})

target_link_libraries(Parser simdjson PluginRegister Primitives dl)
add_dependencies(Parser simdjson PluginRegister)

