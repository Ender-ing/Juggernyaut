# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_configs)
#enable_testing()

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

# Shared directories
set(JUG_BUILD_DIR ${CMAKE_BINARY_DIR})
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.cmake)
set(JUG_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE)
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.deps)
set(JUG_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.dist)

if(NOT DEFINED JUG_GLOBAL_BUILD)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../core ${JUG_BUILD_DIR}/configs_core)
endif()

# Directories
set(JUG_CONFIGS_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_CONFIGS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_CONFIGS_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/manifest.ini)
set(JUG_CONFIGS_MAIN_CPP_PATH ${JUG_CONFIGS_SOURCE_DIR}/main.cpp)

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring Juggernyaut Configs...")

# CMake modules path
set(CMAKE_MODULE_PATH
    # CMake files folder
    ${JUG_CMAKE_DIR}
    ${JUG_CONFIGS_CMAKE_DIR}
    # Build folder
    ${CMAKE_BINARY_DIR}
    # Append existing paths
    ${CMAKE_MODULE_PATH}
)

# Manage dependencies
include(${JUG_CONFIGS_CMAKE_DIR}/dependencies.cmake)

# CMake functions & macros
include(${JUG_CMAKE_DIR}/shared.cmake)

# Build the libraries!
include(${JUG_CONFIGS_CMAKE_DIR}/libraries.cmake)
