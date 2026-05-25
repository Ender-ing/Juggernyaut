# Manage C++ compiler settings
if (NOT DEFINED add_c_cpp_global_flag)
    include(${JUG_CMAKE_DIR}/compiler/cpp.cmake)
endif()

# CMake functions & macros
if (NOT DEFINED target_sources_search)
    include(${JUG_CMAKE_DIR}/utility.cmake)
endif()

# Manage binary output
if (NOT DEFINED JUG_BINARY_MODE)
    include(${JUG_CMAKE_DIR}/build-output.cmake)
endif()

# Manage shared dependencies
include(${JUG_CMAKE_DIR}/dependencies.cmake)

# Libraries
include(${JUG_CMAKE_DIR}/libraries.cmake)

# Diagnostic codes
include(${JUG_CMAKE_DIR}/diagnostics.cmake)
