# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_tests_core)

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)
dep_check()

# Add tests
add_test_target(JugTestsCore ${CMAKE_CURRENT_LIST_DIR})
target_sources_search(JugTestsCore ${CMAKE_CURRENT_LIST_DIR}/parser/*.cpp TRUE)
