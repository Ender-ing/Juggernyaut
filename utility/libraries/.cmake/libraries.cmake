# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

# Create a library from /common
add_library(JuggernyautCommonLibrary SHARED)
target_sources_search(JuggernyautCommonLibrary ${JUG_UTILITY_LIBRARIES_DIR}/common/*.cpp FALSE)
# Expose library exports
target_compile_definitions(JuggernyautCommonLibrary PRIVATE JUG_COMMON_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautCommonLibrary ${JUG_UTILITY_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCommonLibrary FALSE)
# Linking
set(JUG_COMMON_INCLUDE_DIR "${JUG_UTILITY_LIBRARIES_DIR}/common/include" CACHE STRING "Common include path" FORCE)
target_include_directories(JuggernyautCommonLibrary PUBLIC ${JUG_COMMON_INCLUDE_DIR})

# Create a library from /cat-console
add_library(JuggernyautCatConsoleLibrary SHARED)
target_sources_search(JuggernyautCatConsoleLibrary ${JUG_UTILITY_LIBRARIES_DIR}/cat-console/*.cpp FALSE)
# Dependencies
add_dependencies(JuggernyautCatConsoleLibrary JuggernyautCommonLibrary fmt::fmt)
target_link_libraries(JuggernyautCatConsoleLibrary PRIVATE JuggernyautCommonLibrary)
target_link_libraries(JuggernyautCatConsoleLibrary PRIVATE fmt::fmt)
# Expose library exports
target_compile_definitions(JuggernyautCatConsoleLibrary PRIVATE JUG_CAT_CONSOLE_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautCatConsoleLibrary ${JUG_UTILITY_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCatConsoleLibrary FALSE)
# Linking
set(JUG_CAT_CONSOLE_INCLUDE_DIR "${JUG_UTILITY_LIBRARIES_DIR}/cat-console/include" CACHE STRING "Cat Console include path" FORCE)
target_include_directories(JuggernyautCatConsoleLibrary PUBLIC ${JUG_CAT_CONSOLE_INCLUDE_DIR})
