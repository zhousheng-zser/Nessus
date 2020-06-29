file(GLOB parser_source_files ${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/*.cpp 
				${CMAKE_CURRENT_SOURCE_DIR}/../../parser/cpp/jni/*.cpp) 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../3rd-party/cpp/simdjson/include 
			${CMAKE_CURRENT_SOURCE_DIR}/../../register/cpp/PluginRegister 
			${COMMON_INCLUDE} 
			${JNI_INCLUDE})

link_directories(../../../Excalibur/cmake/build/lib 
		/usr/local/cuda/lib64)
add_library(parser SHARED ${parser_source_files})

target_link_libraries(parser simdjson Primitives dl cudart)
add_dependencies(parser simdjson)

