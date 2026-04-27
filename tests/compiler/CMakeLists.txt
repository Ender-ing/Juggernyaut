# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_tests_compiler)
set(JUG_TESTS_COMPILER ON CACHE BOOL "Test tracking" FORCE)

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)
dep_check()

# Add tests
add_test_target(JugTestsCompiler ${CMAKE_CURRENT_LIST_DIR})
target_sources_search(JugTestsCompiler ${CMAKE_CURRENT_LIST_DIR}/flags/*.cpp TRUE)
