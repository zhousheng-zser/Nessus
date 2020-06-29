#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

option(SIMDJSON_ENABLE_THREADS "enable threaded operation" ON)

if(${CMAKE_C_COMPILER_ID} MATCHES "Intel") # icc / icpc
  # prevent shared libraries from depending on Intel provided libraries
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-intel")
endif()

include_directories(../../../3rd-party/cpp/simdjson/src 
			../../../3rd-party/cpp/simdjson/include 
			../../../3rd-party/cpp/simdjson/include/simdjson)
file(GLOB simdjson_source_files ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/simdjson/src/simdjson.cpp 
				${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/simdjson/src/implementtation.cpp 
				${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/simdjson/src/stage1_find_marks.cpp 
				${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/simdjson/src/stage2_build_tape.cpp)

add_library(simdjson STATIC ${simdjson_source_files})

if(SIMDJSON_ENABLE_THREADS)
  find_package(Threads REQUIRED)
  target_link_libraries(simdjson Threads::Threads)
endif()
