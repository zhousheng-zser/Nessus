cmake_minimum_required(VERSION 3.14.3)

set(target_name license_client)
set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../license/cpp/client)
set(common_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../license/cpp)
set(3rd_party_dir ${source_dir}/../../../3rd-party/cpp)

if(WITH_JNI OR ANDROID)
	file(GLOB_RECURSE source_files ${source_dir}/*.cpp ${source_dir}/*.hpp)
else()
	file(GLOB source_files ${source_dir}/*.cpp ${source_dir}/*.hpp)
endif()

add_library(${target_name} SHARED ${source_files})

target_include_directories(${target_name} PRIVATE ${source_dir} ${common_lib_dir} ${3rd_party_dir}/asio/include ${3rd_party_dir}/websocketpp/include ${3rd_party_dir}/nlohmann/include ${3rd_party_dir}/openssl/include ${COMMON_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS})
target_compile_definitions(${target_name} PRIVATE -DEXPORT_NESSUS_LICENSE -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STRICT_)
target_link_libraries(${target_name} PRIVATE ssl crypto license-common-lib ${COMMON_LIBRARIES})
add_dependencies(${target_name} ssl crypto license-common-lib)

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
	target_link_directories(${target_name} PRIVATE ${COMMON_LIBRARY_DIRS})
endif()
