# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_tests_server)
set(JUG_TESTS_SERVER ON CACHE BOOL "Test tracking" FORCE)

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)
dep_check()

# Add tests
add_test_target(JugTestsServer ${CMAKE_CURRENT_LIST_DIR})
target_sources_search(JugTestsServer ${CMAKE_CURRENT_LIST_DIR}/connection/*.cpp TRUE)
