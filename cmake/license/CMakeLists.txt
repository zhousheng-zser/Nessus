cmake_minimum_required(VERSION 3.14.3)

set(target_name license-common-lib)
set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../license/cpp)
set(3rd_party_dir ${source_dir}/../../3rd-party/cpp)

file(GLOB source_files ${source_dir}/*.cpp ${source_dir}/*.hpp)
add_library(${target_name} ${source_files})

target_include_directories(${target_name} PRIVATE ${3rd_party_dir}/asio/include ${3rd_party_dir}/nlohmann/include ${3rd_party_dir}/openssl/include ${COMMON_INCLUDE_DIRS})
target_link_libraries(${target_name} PRIVATE crypto ${COMMON_LIBRARIES})
add_dependencies(${target_name} crypto)
