# Manage C++ compiler settings
include(${JUG_CMAKE_DIR}/compiler/cpp.cmake)

# CMake functions & macros
include(${JUG_CMAKE_DIR}/utility.cmake)

# Manage binary output
include(${JUG_CMAKE_DIR}/build-output.cmake)

# Manage shared dependencies
include(${JUG_CMAKE_DIR}/dependencies.cmake)

# CMake files that are not related to the main building process
include(${JUG_CMAKE_DIR}/other/legal.cmake)

# Diagnostic codes
include(${JUG_CMAKE_DIR}/diagnostics.cmake)
