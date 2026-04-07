# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_core)
#enable_testing()

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

# Shared directories
set(JUG_BUILD_DIR ${CMAKE_BINARY_DIR})
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.cmake)
set(JUG_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE)
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.deps)
set(JUG_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.dist)

# Directories
set(JUG_CORE_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_CORE_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/manifest.ini)

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring Juggernyaut Core...")

# CMake modules path
set(CMAKE_MODULE_PATH
    # CMake files folder
    ${JUG_CMAKE_DIR}
    ${JUG_CMAKE_DIR}/ANTLR4
    ${JUG_CORE_CMAKE_DIR}
    # Build folder
    ${CMAKE_BINARY_DIR}
    # Append existing paths
    ${CMAKE_MODULE_PATH}
)

# ANTLR4 files generation configuration
set(JUG_ANTLR4_LEXER_PATH ${JUG_CORE_SOURCE_DIR}/parser/.antlr4/JuggernyautLexer.g4)
set(JUG_ANTLR4_PARSER_PATH ${JUG_CORE_SOURCE_DIR}/parser/.antlr4/JuggernyautParser.g4)

# Manage dependencies
include(${JUG_CORE_CMAKE_DIR}/dependencies.cmake)

# CMake functions & macros
include(${JUG_CMAKE_DIR}/shared.cmake)

# Generate ANTLR4
include(${JUG_CORE_CMAKE_DIR}/ANTLR4-gen.cmake)

# Generate Core libraries
include(${JUG_CORE_CMAKE_DIR}/libraries.cmake)
