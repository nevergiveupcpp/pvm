cmake_minimum_required(VERSION 3.15)
project(pvm VERSION 1.0.0 LANGUAGES CXX)

add_library(pvm INTERFACE)
add_library(pvm::pvm ALIAS pvm)

target_include_directories(
        pvm INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(pvm INTERFACE cxx_std_20)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dev.cmake")
    include(dev.cmake)
endif()

install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

install(TARGETS pvm
        EXPORT pvmTargets
)

install(EXPORT pvmTargets
        FILE pvmTargets.cmake
        NAMESPACE pvm::
        DESTINATION lib/cmake/pvm
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pvmConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/pvmConfig.cmake
        INSTALL_DESTINATION lib/cmake/pvm
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/pvmConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/pvmConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/pvmConfigVersion.cmake
        DESTINATION lib/cmake/pvm
)