# Add executable and include relevant files
message(STATUS "[BUILD] Adding executable target 'JugDiagnosticsCode'...")
add_executable(
    JugDiagnosticsCode ${JUG_UTILITY_DIAGCODES_MAIN_CPP_PATH}
    ${ANTLR_DiagnosticsCodeLexer_CXX_OUTPUTS} # ANTLR4
    ${ANTLR_DiagnosticsCodeParser_CXX_OUTPUTS} # ANTLR4
)

# Dependencies 
add_dependencies(JugDiagnosticsCode antlr4_static)
target_link_libraries(JugDiagnosticsCode PRIVATE antlr4_static)
target_compile_definitions(JugDiagnosticsCode PRIVATE ANTLR4CPP_STATIC)

target_include_directories(JugDiagnosticsCode SYSTEM PRIVATE ${ANTLR4_INCLUDE_DIRS})
target_include_directories(JugDiagnosticsCode SYSTEM PRIVATE ${ANTLR_DiagnosticsCodeLexer_OUTPUT_DIR})
target_include_directories(JugDiagnosticsCode SYSTEM PRIVATE ${ANTLR_DiagnosticsCodeParser_OUTPUT_DIR})
