# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_utility_libraries)

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

include(FetchContent)

# Shared directories
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../.deps)
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../.cmake)

# Directories
set(JUG_UTILITY_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_UTILITY_LIBRARIES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_UTILITY_LIBRARIES_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_UTILITY_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../manifest.ini)

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring the Common Library...")

# CMake modules path
set(CMAKE_MODULE_PATH
    # CMake files folder
    ${JUG_UTILITY_LIBRARIES_DIR}
    # Build folder
    ${CMAKE_BINARY_DIR}
    # Append existing paths
    ${CMAKE_MODULE_PATH}
)

# Manage C++ compiler settings
include(${JUG_CMAKE_DIR}/compiler/cpp.cmake)

# Manage dependencies
include(${JUG_UTILITY_CMAKE_DIR}/dependencies.cmake)
include(${JUG_CMAKE_DIR}/dependencies.cmake)

# Generate libraries
include(${JUG_UTILITY_LIBRARIES_CMAKE_DIR}/libraries.cmake)
