cmake_minimum_required(VERSION 3.14.3)

set(target_name license_server)
set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../license/cpp/server)
set(common_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../license/cpp)
set(3rd_party_dir ${source_dir}/../../../3rd-party/cpp)

file(GLOB_RECURSE source_files ${source_dir}/*.cpp ${source_dir}/*.hpp)
add_executable(${target_name} ${source_files})

target_include_directories(${target_name} PRIVATE ${common_lib_dir} ${3rd_party_dir}/asio/include ${3rd_party_dir}/websocketpp/include ${3rd_party_dir}/nlohmann/include ${3rd_party_dir}/openssl/include ${3rd_party_dir}/libpqxx/include ${COMMON_INCLUDE_DIRS})
target_compile_definitions(${target_name} PRIVATE -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STRICT_)
target_link_libraries(${target_name} PRIVATE ssl crypto pqxx license-common-lib ${COMMON_LIBRARIES})
add_dependencies(${target_name} ssl crypto pqxx license-common-lib)
