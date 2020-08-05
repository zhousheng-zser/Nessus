file(GLOB parser_source_files ${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/*.cpp 
				${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/jni/*.cpp) 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../3rd-party/cpp/simdjson/singleheader 
			${CMAKE_CURRENT_SOURCE_DIR}/../../register/cpp/PluginRegister 
			${COMMON_INCLUDE} 
			${JNI_INCLUDE})

link_directories(${COMMON_LIB_PATH})
add_library(Parser SHARED ${parser_source_files})

target_link_libraries(Parser simdjson PluginRegister Primitives dl stdc++fs)
add_dependencies(Parser simdjson PluginRegister)

