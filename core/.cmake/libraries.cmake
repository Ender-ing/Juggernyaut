# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

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
add_dependencies(JuggernyautDiagnosticsLibrary antlr4_shared JugGlobalDiagnostics)
target_link_libraries(JuggernyautDiagnosticsLibrary PUBLIC antlr4_shared JugGlobalDiagnostics)

# Create a library from /data
add_library(JuggernyautDataLibrary SHARED)
target_sources_search(JuggernyautDataLibrary ${JUG_CORE_SOURCE_DIR}/data/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautDataLibrary PRIVATE JUG_DATA_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautDataLibrary ${JUG_CORE_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautDataLibrary FALSE)
# Dependencies
add_dependencies(JuggernyautDataLibrary JugGlobalDiagnostics)
target_link_libraries(JuggernyautDataLibrary PUBLIC JugGlobalDiagnostics)

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
add_dependencies(JuggernyautParserLibrary antlr4_shared JuggernyautDiagnosticsLibrary JuggernyautDataLibrary)
target_link_libraries(JuggernyautParserLibrary PUBLIC antlr4_shared JuggernyautDiagnosticsLibrary JuggernyautDataLibrary)
# Libraries
copy_shared_library(JuggernyautParserLibrary ${ANTLR4_RUNTIME_LIBRARIES} ${ANTLR4_TAG})

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
add_dependencies(JuggernyautSessionLibrary JuggernyautDiagnosticsLibrary JugGlobalDiagnostics JuggernyautDataLibrary JuggernyautParserLibrary)
target_link_libraries(JuggernyautSessionLibrary PUBLIC JuggernyautDiagnosticsLibrary JugGlobalDiagnostics JuggernyautDataLibrary JuggernyautParserLibrary)
