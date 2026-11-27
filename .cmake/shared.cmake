include(${JUG_CMAKE_DIR}/shared-cmake.cmake)

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
