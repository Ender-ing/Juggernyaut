# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

# Create a library
add_library(JuggernyautConfigsLibrary SHARED)
target_sources_search(JuggernyautConfigsLibrary ${JUG_CONFIGS_SOURCE_DIR}/*.cpp FALSE)
# Expose library exports
target_compile_definitions(JuggernyautConfigsLibrary PRIVATE JUG_CONFIGS_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautConfigsLibrary ${JUG_CONFIGS_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautConfigsLibrary FALSE)
# Dependencies
add_dependencies(JuggernyautConfigsLibrary JuggernyautSessionLibrary)
target_link_libraries(JuggernyautConfigsLibrary PUBLIC JuggernyautSessionLibrary)
target_include_directories(JuggernyautConfigsLibrary PRIVATE ${TOMLPP_INCLUDE_DIR})
