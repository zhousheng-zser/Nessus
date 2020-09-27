cmake_minimum_required(VERSION 3.14.3)

set(libpq_version 7.2.1)

if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd-party/cpp/libpqxx)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(config.cmake)

add_subdirectory(src)
#add_subdirectory(include)

# installation
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libpqxx-config-version.cmake"
    VERSION ${libpq_version}
    COMPATIBILITY SameMajorVersion
)

# Build tree export
export(
    EXPORT libpqxx-targets
    NAMESPACE libpqxx::
    FILE ${CMAKE_CURRENT_BINARY_DIR}/libpqxx-targets.cmake
)
configure_file(
    libpqxx-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/libpqxx-config.cmake
    COPYONLY
)
# Package generation
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)
