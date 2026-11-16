# Locate npm (on Windows, npm is npm.cmd)
find_program(NPM_EXECUTABLE NAMES npm.cmd npm REQUIRED)

set(JUG_TS_GRAMMAR_DIR "${JUG_MAIN_DIR}/extensions/tree-sitter-jug")
set(JUG_TS_PARSER_C    "${JUG_TS_GRAMMAR_DIR}/src/parser.c")
set(JUG_TS_PARSER_H    "${JUG_TS_GRAMMAR_DIR}/src/tree_sitter/parser.h")

# Track all grammar JS files so CMake re-runs when any of them change
file(GLOB_RECURSE JUG_TS_GRAMMAR_SOURCES CONFIGURE_DEPENDS
    "${JUG_TS_GRAMMAR_DIR}/grammar/*.js"
)
list(APPEND JUG_TS_GRAMMAR_SOURCES
    "${JUG_TS_GRAMMAR_DIR}/grammar.js"
    "${JUG_TS_GRAMMAR_DIR}/package.json"
)

# Ensure node_modules is installed at configure time if missing
if(NOT EXISTS "${JUG_TS_GRAMMAR_DIR}/node_modules")
    message(STATUS "[TREE-SITTER] Installing npm dependencies in ${JUG_TS_GRAMMAR_DIR}...")
    execute_process(
        COMMAND "${NPM_EXECUTABLE}" install
        WORKING_DIRECTORY "${JUG_TS_GRAMMAR_DIR}"
        RESULT_VARIABLE _npm_install_res
    )
    if(NOT _npm_install_res EQUAL 0)
        message(FATAL_ERROR "[TREE-SITTER] 'npm install' failed in ${JUG_TS_GRAMMAR_DIR}")
    endif()
endif()

# Generate src/parser.c whenever any grammar .js file changes
add_custom_command(
    OUTPUT  "${JUG_TS_PARSER_C}" "${JUG_TS_PARSER_H}"
    COMMAND "${NPM_EXECUTABLE}" run generate:parser
    WORKING_DIRECTORY "${JUG_TS_GRAMMAR_DIR}"
    DEPENDS ${JUG_TS_GRAMMAR_SOURCES}
    COMMENT "[TREE-SITTER] Generating Juggernyaut parser (src/parser.c)..."
    VERBATIM
)

add_custom_target(JuggernyautTreeSitterGen
    DEPENDS "${JUG_TS_PARSER_C}" "${JUG_TS_PARSER_H}"
)
