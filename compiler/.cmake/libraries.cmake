# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

# Create a library from /console
add_library(JugCompilerConsoleLibrary STATIC)
target_sources_search(JugCompilerConsoleLibrary ${JUG_COMPILER_SOURCE_DIR}/console/*.cpp FALSE)
# Attach manifest data
attach_manifest_data(JugCompilerConsoleLibrary ${JUG_COMPILER_MANIFEST_FILE} FALSE)
# Add compiler flags
add_internal_target_cxx_flags(JugCompilerConsoleLibrary FALSE)
# Link other libraries to the library
add_dependencies(JugCompilerConsoleLibrary fmt::fmt)
target_link_libraries(JugCompilerConsoleLibrary PUBLIC fmt::fmt)

# Create a library from /store
add_library(JugCompilerStoreLibrary STATIC)
target_sources_search(JugCompilerStoreLibrary ${JUG_COMPILER_SOURCE_DIR}/store/*.cpp FALSE)
# Attach manifest data
attach_manifest_data(JugCompilerStoreLibrary ${JUG_COMPILER_MANIFEST_FILE} FALSE)
# Add compiler flags
add_internal_target_cxx_flags(JugCompilerStoreLibrary FALSE)
# Link other libraries to the library
add_dependencies(JugCompilerStoreLibrary JugCompilerConsoleLibrary JuggernyautDataLibrary)
target_link_libraries(JugCompilerStoreLibrary PUBLIC JugCompilerConsoleLibrary JuggernyautDataLibrary)

# Create a library from /base
add_library(JugCompilerBaseLibrary STATIC)
target_sources_search(JugCompilerBaseLibrary ${JUG_COMPILER_SOURCE_DIR}/base/*.cpp FALSE)
# Attach manifest data
attach_manifest_data(JugCompilerBaseLibrary ${JUG_COMPILER_MANIFEST_FILE} FALSE)
# Add compiler flags
add_internal_target_cxx_flags(JugCompilerBaseLibrary FALSE)
# Link other libraries to the library
add_dependencies(JugCompilerBaseLibrary JugCompilerStoreLibrary)
target_link_libraries(JugCompilerBaseLibrary PUBLIC JugCompilerStoreLibrary)

# Compiler libraries
set(COMPILER_LIBRARIES
    JugCompilerBaseLibrary
    JugCompilerConsoleLibrary
    JugCompilerStoreLibrary
)