# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

# Create a library from /common
add_library(JuggernyautCommonLibrary SHARED)
target_sources_search(JuggernyautCommonLibrary ${JUG_UTILITY_COMMON_DIR}/*.cpp FALSE)
# Expose library exports
target_compile_definitions(JuggernyautCommonLibrary PRIVATE JUG_COMMON_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautCommonLibrary ${JUG_UTILITY_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCommonLibrary FALSE)
# Libraries
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    copy_proper_shared_library(JuggernyautCommonLibrary $<TARGET_FILE_DIR:mimalloc> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# Linking
set(JUG_COMMON_INCLUDE_DIR "${JUG_UTILITY_COMMON_DIR}/include" CACHE STRING "Common include path" FORCE)
target_include_directories(JuggernyautCommonLibrary PUBLIC ${JUG_COMMON_INCLUDE_DIR})
