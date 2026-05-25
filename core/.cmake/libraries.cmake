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
jug_common(JuggernyautDiagnosticsLibrary)
add_dependencies(JuggernyautDiagnosticsLibrary antlr4_shared JugGlobalDiagnostics)
target_link_libraries(JuggernyautDiagnosticsLibrary PUBLIC JugGlobalDiagnostics antlr4_shared)

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
jug_common(JuggernyautDataLibrary)
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
jug_common(JuggernyautParserLibrary)
add_dependencies(JuggernyautParserLibrary antlr4_shared JuggernyautDiagnosticsLibrary JuggernyautDataLibrary)
target_link_libraries(JuggernyautParserLibrary PUBLIC JuggernyautDiagnosticsLibrary JuggernyautDataLibrary antlr4_shared)
# Handle known issues
if(MSVC)
    # 4275: non-exported base class
    # 4251: non-exported member variable
    target_compile_options(JuggernyautParserLibrary PRIVATE /wd4275 /wd4251)
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
jug_common(JuggernyautSessionLibrary)
add_dependencies(JuggernyautSessionLibrary JuggernyautDiagnosticsLibrary JuggernyautDataLibrary JuggernyautParserLibrary)
target_link_libraries(JuggernyautSessionLibrary PUBLIC JuggernyautDiagnosticsLibrary JuggernyautDataLibrary JuggernyautParserLibrary)

# Create a library from /manager
add_library(JuggernyautManagerLibrary SHARED)
target_sources_search(JuggernyautManagerLibrary ${JUG_CORE_SOURCE_DIR}/manager/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautManagerLibrary PRIVATE JUG_MANAGER_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautManagerLibrary ${JUG_CORE_MANIFEST_FILE} TRUE)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautManagerLibrary FALSE)
# Dependencies
jug_common(JuggernyautManagerLibrary)
add_dependencies(JuggernyautManagerLibrary JuggernyautDiagnosticsLibrary JuggernyautDataLibrary JuggernyautSessionLibrary)
target_link_libraries(JuggernyautManagerLibrary PUBLIC JuggernyautDiagnosticsLibrary JuggernyautDataLibrary JuggernyautSessionLibrary)
