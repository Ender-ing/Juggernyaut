# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

# Create a library from /common
add_library(JuggernyautCommonLibrary SHARED)
target_sources_search(JuggernyautCommonLibrary ${JUG_CORE_SOURCE_DIR}/common/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautCommonLibrary PRIVATE JUG_COMMON_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautCommonLibrary ${JUG_CORE_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCommonLibrary FALSE)

# Create a library from /diagnostics
add_library(JuggernyautDiagnosticsLibrary SHARED)
target_sources_search(JuggernyautDiagnosticsLibrary ${JUG_CORE_SOURCE_DIR}/diagnostics/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautDiagnosticsLibrary PRIVATE JUG_DIAGNOSTICS_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautDiagnosticsLibrary ${JUG_CORE_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautDiagnosticsLibrary FALSE)
# Dependencies
add_dependencies(JuggernyautDiagnosticsLibrary antlr4_shared JuggernyautCommonLibrary JugGlobalDiagnostics)
target_link_libraries(JuggernyautDiagnosticsLibrary PUBLIC antlr4_shared JuggernyautCommonLibrary JugGlobalDiagnostics)


# Create a library from /store
add_library(JuggernyautStoreLibrary SHARED)
target_sources_search(JuggernyautStoreLibrary ${JUG_CORE_SOURCE_DIR}/store/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautStoreLibrary PRIVATE JUG_STORE_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautStoreLibrary ${JUG_CORE_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautStoreLibrary FALSE)
# Dependencies
add_dependencies(JuggernyautStoreLibrary JuggernyautCommonLibrary JugGlobalDiagnostics)
target_link_libraries(JuggernyautStoreLibrary PUBLIC JuggernyautCommonLibrary JugGlobalDiagnostics)

# Create a library from /parser
add_library(JuggernyautParserLibrary SHARED
    ${ANTLR_JuggernyautGrammarLexer_CXX_OUTPUTS} # ANTLR4
    ${ANTLR_JuggernyautGrammarParser_CXX_OUTPUTS} # ANTLR4
)
target_sources_search(JuggernyautParserLibrary ${JUG_CORE_SOURCE_DIR}/parser/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautParserLibrary PRIVATE JUG_PARSER_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautParserLibrary ${JUG_CORE_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautParserLibrary TRUE)
# Dependencies
add_dependencies(JuggernyautParserLibrary antlr4_shared JuggernyautDiagnosticsLibrary JuggernyautStoreLibrary)
target_link_libraries(JuggernyautParserLibrary PUBLIC antlr4_shared JuggernyautDiagnosticsLibrary JuggernyautStoreLibrary)
# Libraries
add_custom_command(TARGET JuggernyautParserLibrary
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND}
                           -E copy ${ANTLR4_RUNTIME_LIBRARIES} .
                    WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
# Fix antlr4-runtime library naming!
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    get_filename_component(ANTLR_FILENAME "${ANTLR4_RUNTIME_LIBRARIES}" NAME)
    delete_file(JuggernyautParserLibrary "${ANTLR_FILENAME}.${ANTLR4_TAG}")
    rename_file(JuggernyautParserLibrary ${ANTLR_FILENAME} "${ANTLR_FILENAME}.${ANTLR4_TAG}")
    create_symbolic_link(JuggernyautParserLibrary "${ANTLR_FILENAME}.${ANTLR4_TAG}" ${ANTLR_FILENAME} FALSE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    get_filename_component(ANTLR_FILENAME_NOEXT "${ANTLR4_RUNTIME_LIBRARIES}" NAME_WE)
    delete_file(JuggernyautParserLibrary "${ANTLR_FILENAME_NOEXT}.${ANTLR4_TAG}.dylib")
    rename_file(JuggernyautParserLibrary "${ANTLR_FILENAME_NOEXT}.dylib" "${ANTLR_FILENAME_NOEXT}.${ANTLR4_TAG}.dylib")
    create_symbolic_link(JuggernyautParserLibrary "${ANTLR_FILENAME_NOEXT}.${ANTLR4_TAG}.dylib" "${ANTLR_FILENAME_NOEXT}.dylib" FALSE)
endif()

# Create a library from /session
add_library(JuggernyautSessionLibrary SHARED)
target_sources_search(JuggernyautSessionLibrary ${JUG_CORE_SOURCE_DIR}/session/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautSessionLibrary PRIVATE JUG_SESSION_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautSessionLibrary ${JUG_CORE_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautSessionLibrary FALSE)
# Dependencies
add_dependencies(JuggernyautSessionLibrary JuggernyautCommonLibrary JuggernyautDiagnosticsLibrary JugGlobalDiagnostics JuggernyautStoreLibrary JuggernyautParserLibrary)
target_link_libraries(JuggernyautSessionLibrary PUBLIC JuggernyautCommonLibrary JuggernyautDiagnosticsLibrary JugGlobalDiagnostics JuggernyautStoreLibrary JuggernyautParserLibrary)

# Expose the core libraries
set(CORE_LIBRARIES
    JuggernyautCommonLibrary
    JuggernyautDiagnosticsLibrary
    JuggernyautStoreLibrary
    JuggernyautParserLibrary
    JuggernyautSessionLibrary
    PARENT_SCOPE
)
