# Basic info
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(juggernyaut_compiler)
#enable_testing()

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

include(FetchContent)

# Directories
set(JUG_BUILD_DIR ${CMAKE_BINARY_DIR})
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.deps)
set(JUG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.dist)
set(JUG_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/manifest.ini)
set(JUG_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE)
set(JUG_MAIN_CPP_PATH ${JUG_SOURCE_DIR}/main.cpp)

# CMake modules path
set(CMAKE_MODULE_PATH
    # CMake files folder
    ${JUG_CMAKE_DIR}/ANTLR4
    ${JUG_CMAKE_DIR}
    # Build folder
    ${CMAKE_BINARY_DIR}
    # Append existing paths
    ${CMAKE_MODULE_PATH}
)

# ANTLR4 files generation configuration
#

# CMake functions & macros
include(${JUG_CMAKE_DIR}/utility.cmake)

# Manage C++ compiler settings
include(${JUG_CMAKE_DIR}/compiler/cpp.cmake)

# Manage binary output
include(${JUG_CMAKE_DIR}/build-output.cmake)

# Manage dependencies
include(${JUG_CMAKE_DIR}/dependencies.cmake)

# Build the compiler executable!
include(${JUG_CMAKE_DIR}/build-executable.cmake)

# CMake files that are not related to the main building process
include(${JUG_CMAKE_DIR}/other/legal.cmake)
