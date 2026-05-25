# Basic info
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(juggernyaut_tests_package_manager)
set(JUG_TESTS_PACKAGE_MANAGER ON CACHE BOOL "Test tracking" FORCE)

# Stop build when errors occur
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)
dep_check()

# Add tests
add_test_target(JugTestsPackageManager ${CMAKE_CURRENT_LIST_DIR})
#target_sources_search(JugTestsPackageManager ${CMAKE_CURRENT_LIST_DIR}/???/*.cpp TRUE)
