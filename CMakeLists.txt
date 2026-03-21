# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut)
#enable_testing()

# Shared directories
set(JUG_BUILD_DIR ${CMAKE_BINARY_DIR})
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.deps)
set(JUG_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.dist)

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring Juggernyaut...")

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

include(${JUG_CMAKE_DIR}/shared.cmake)

set(JUG_GLOBAL_BUILD ON)
add_subdirectory(core)
add_subdirectory(compiler)
add_subdirectory(server)

# Delete leftover files
if(${JUG_BINARY_MODE} STREQUAL "Release")
    add_custom_target(PostBuildTask ALL
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
        COMMENT "Post-build cleanup..."
    )
    add_dependencies(PostBuildTask
        JuggernyautCompiler # Target from compiler
        JuggernyautServer # Target from server
    )
endif()
