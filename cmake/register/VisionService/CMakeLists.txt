file(GLOB vision_service_source_files ${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/VisionService/dllmain.cpp 
					${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/VisionService/vision_service.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/VisionService 
			${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister 
			${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp 
			${COMMON_INCLUDE})

add_library(VisionService SHARED ${vision_service_source_files})


