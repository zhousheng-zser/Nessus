file(GLOB vision_service_source_files ${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/VisionService/dllmain.cpp 
					${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/VisionService/vision_service.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/VisionService 
			${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp/PluginRegister 
			${CMAKE_CURRENT_SOURCE_DIR}/../../../register/cpp 
			${COMMON_INCLUDE})

link_directories(${VISION_LIBRARY_DIRECTORIES})

add_library(VisionService SHARED ${vision_service_source_files})

target_link_libraries(VisionService PluginRegister Primitives Longinus Cassius Gaius Irisviel)
add_dependencies(VisionService PluginRegister)
