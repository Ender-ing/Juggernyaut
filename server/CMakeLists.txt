# Basic info
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(juggernyaut_server)
#enable_testing()

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

include(FetchContent)

# Shared directories
set(JUG_BUILD_DIR ${CMAKE_BINARY_DIR})
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.cmake)
set(JUG_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE)
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.deps)
set(JUG_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.dist)

# Directories
set(JUG_SERVER_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_SERVER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_SERVER_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/manifest.ini)
#set(JUG_SERVER_MAIN_CPP_PATH ${JUG_SERVER_SOURCE_DIR}/main.cpp)

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring Juggernyaut Language Server...")

# CMake modules path
set(CMAKE_MODULE_PATH
    # CMake files folder
    ${JUG_CMAKE_DIR}
    ${JUG_SERVER_CMAKE_DIR}
    # Build folder
    ${CMAKE_BINARY_DIR}
    # Append existing paths
    ${CMAKE_MODULE_PATH}
)

# CMake functions & macros
include(${JUG_CMAKE_DIR}/shared.cmake)

# Manage dependencies
include(${JUG_SERVER_CMAKE_DIR}/dependencies.cmake)
