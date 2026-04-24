# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_tests)
set(JUG_TESTS_BUILD ON)

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

# Shared directories
set(JUG_BUILD_DIR ${CMAKE_BINARY_DIR})
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.cmake)
set(JUG_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE)
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.deps)
set(JUG_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.dist)

# Directories
set(JUG_TESTS_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_TESTS_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/manifest.ini)

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring Juggernyaut tests...")

# CMake modules path
set(CMAKE_MODULE_PATH
    # CMake files folder
    ${JUG_CMAKE_DIR}
    ${JUG_TESTS_CMAKE_DIR}
    # Build folder
    ${CMAKE_BINARY_DIR}
    # Append existing paths
    ${CMAKE_MODULE_PATH}
)

# CMake functions & macros
include(${JUG_CMAKE_DIR}/shared.cmake)

# Manage binary output
include(${JUG_TESTS_CMAKE_DIR}/build-output.cmake)

# Manage dependencies
include(${JUG_TESTS_CMAKE_DIR}/dependencies.cmake)

# Test utility
include(${JUG_TESTS_CMAKE_DIR}/utility.cmake)

# Build the executables!
#include(${JUG_TESTS_CMAKE_DIR}/build-executable.cmake)

if((DEFINED JUG_GLOBAL_BUILD) OR (DEFINED JUG_CORE_BUILD))
    message(STATUS "[TESTS] Configuring 'core' tests...")
    #add_subdirectory(${JUG_TESTS_SOURCE_DIR}/core ${JUG_BUILD_DIR}/tests_core)
endif()
if((DEFINED JUG_GLOBAL_BUILD) OR (DEFINED JUG_CONFIGS_BUILD))
    message(STATUS "[TESTS] Configuring 'configs' tests...")
    #add_subdirectory(${JUG_TESTS_SOURCE_DIR}/configs ${JUG_BUILD_DIR}/tests_configs)
endif()
if((DEFINED JUG_GLOBAL_BUILD) OR (DEFINED JUG_COMPILER_BUILD))
    message(STATUS "[TESTS] Configuring 'compiler' tests...")
    #add_subdirectory(${JUG_TESTS_SOURCE_DIR}/compiler ${JUG_BUILD_DIR}/tests_compiler)
endif()
if((DEFINED JUG_GLOBAL_BUILD) OR (DEFINED JUG_SERVER_BUILD))
    message(STATUS "[TESTS] Configuring 'server' tests...")
    #add_subdirectory(${JUG_TESTS_SOURCE_DIR}/server ${JUG_BUILD_DIR}/tests_server)
endif()
