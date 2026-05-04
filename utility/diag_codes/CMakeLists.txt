# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_utility_diagcodes)

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

include(FetchContent)

# Shared directories
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../.deps)
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../.cmake)

# Directories
set(JUG_UTILITY_DIAGCODES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_UTILITY_DIAGCODES_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring Utility DiagCodes...")

# CMake modules path
set(CMAKE_MODULE_PATH
    # CMake files folder
    ${JUG_CMAKE_DIR}/ANTLR4
    ${JUG_UTILITY_DIAGCODES_DIR}
    # Build folder
    ${CMAKE_BINARY_DIR}
    # Append existing paths
    ${CMAKE_MODULE_PATH}
)

set(JUG_UTILITY_DIAGCODES_MAIN_CPP_PATH ${JUG_UTILITY_DIAGCODES_DIR}/main.cpp)

# ANTLR4 files generation configuration
set(JUG_UTILITY_DIAGCODES_ANTLR4_LEXER_PATH ${JUG_UTILITY_DIAGCODES_DIR}/.antlr4/DiagnosticsCodeLexer.g4)
set(JUG_UTILITY_DIAGCODES_ANTLR4_PARSER_PATH ${JUG_UTILITY_DIAGCODES_DIR}/.antlr4/DiagnosticsCodeParser.g4)

# Manage C++ compiler settings
include(${JUG_CMAKE_DIR}/compiler/cpp.cmake)

# Manage dependencies
include(${JUG_UTILITY_DIAGCODES_CMAKE_DIR}/dependencies.cmake)
include(${JUG_CMAKE_DIR}/dependencies.cmake)

# Generate ANTLR4
include(${JUG_UTILITY_DIAGCODES_CMAKE_DIR}/ANTLR4-gen.cmake)

# Generate Core libraries
include(${JUG_UTILITY_DIAGCODES_CMAKE_DIR}/build-executable.cmake)
