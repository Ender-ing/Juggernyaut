# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_package_manager)
set(JUG_PACKAGE_MANAGER_BUILD ON CACHE BOOL "Jug dependency tracking" FORCE)

if(NOT DEFINED JUG_GLOBAL_BUILD)
    # Tests
    option(JUG_BUILD_TESTS "Build the test suite" OFF)
    if(JUG_BUILD_TESTS)
        enable_testing()
    endif()
endif()

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

# Shared directories
set(JUG_BUILD_DIR ${CMAKE_BINARY_DIR})
set(JUG_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.cmake)
set(JUG_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.deps)
set(JUG_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.out)

# Directories
set(JUG_PACKAGE_MANAGER_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
set(JUG_PACKAGE_MANAGER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(JUG_PACKAGE_MANAGER_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/manifest.ini)
set(JUG_PACKAGE_MANAGER_MAIN_CPP_PATH ${JUG_PACKAGE_MANAGER_SOURCE_DIR}/main.cpp)

include(${JUG_CMAKE_DIR}/messages.cmake)
info("[START] Configuring Juggernyaut Package Manager...")

# CMake modules path
set(CMAKE_MODULE_PATH
    # CMake files folder
    ${JUG_CMAKE_DIR}
    ${JUG_PACKAGE_MANAGER_CMAKE_DIR}
    # Build folder
    ${CMAKE_BINARY_DIR}
    # Append existing paths
    ${CMAKE_MODULE_PATH}
)

# Manage dependencies
include(${JUG_PACKAGE_MANAGER_CMAKE_DIR}/dependencies.cmake)

# CMake functions & macros
include(${JUG_CMAKE_DIR}/shared.cmake)

# Build the package manager executable!
include(${JUG_PACKAGE_MANAGER_CMAKE_DIR}/build-executable.cmake)

if(NOT DEFINED JUG_GLOBAL_BUILD)
    if(JUG_BUILD_TESTS)
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../tests ${JUG_BUILD_DIR}/package_manager_tests)
    endif()
endif()
