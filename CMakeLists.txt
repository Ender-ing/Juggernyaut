# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_toolchain)

# Install checks
option(JUG_MATCH_INSTALL_ARCH "Run a test on install that checks the architecture of all installed binaries." OFF)

# Tests
option(JUG_BUILD_TESTS "Build the test suite" OFF)
if(JUG_BUILD_TESTS)
    enable_testing()
endif()

# Shared directories
set(JUG_BUILD_DIR ${CMAKE_BINARY_DIR})
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.deps)
set(JUG_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.out)

# Shared build dir
set(JUG_MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.dist)
set(JUG_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)

# Set a default JUG_RELEASE_VERSION
if(NOT DEFINED JUG_RELEASE_VERSION)
    set(JUG_RELEASE_VERSION "0.0.0-danger.000")
endif()

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring Juggernyaut...")

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

# Handle install configs
include(${JUG_CMAKE_DIR}/compiler/cpp.cmake)
include(${JUG_CMAKE_DIR}/utility.cmake)
include(${JUG_CMAKE_DIR}/build-output.cmake)
include(${JUG_CMAKE_DIR}/pre-install.cmake)

include(${JUG_CMAKE_DIR}/shared.cmake)

# ONLY DIRECT TOOLCHAIN PROJECTS ARE ALLOWED HERE!
set(JUG_GLOBAL_BUILD ON)
add_subdirectory(core)
add_subdirectory(configs)
add_subdirectory(compiler)
add_subdirectory(server)
add_subdirectory(package-manager)

include(${JUG_CMAKE_DIR}/install.cmake)
include(${JUG_CMAKE_DIR}/package.cmake)

# Tests
option(JUG_BUILD_TESTS "Build the test suite" OFF)
if(JUG_BUILD_TESTS)
    add_subdirectory(tests)
endif()
