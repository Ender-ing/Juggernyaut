# Add executable and include relevant files
message(STATUS "[BUILD] Adding executable target 'JugDiagnosticsCode'...")
add_executable(
    JugDiagnosticsCode ${JUG_UTILITY_DIAGCODES_MAIN_CPP_PATH}
    ${ANTLR_DiagnosticsCodeLexer_CXX_OUTPUTS} # ANTLR4
    ${ANTLR_DiagnosticsCodeParser_CXX_OUTPUTS} # ANTLR4
)

# ANTLR4
add_dependencies(JugDiagnosticsCode antlr4_static)
target_link_libraries(JugDiagnosticsCode PUBLIC antlr4_static)
target_compile_definitions(JugDiagnosticsCode PRIVATE ANTLR4CPP_STATIC)
