# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

# required if linking to static library
# add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# WARNING!
# Patches may be applied to the ANTLR4 runtime source code!
# Make sure to update the patch files if the version changes!

# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antlr4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE ${JUG_DEPENDENCIES_ANTLR4_JAR_PATH})
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

# Call macro to add lexer and grammar to your build dependencies.
antlr_target(JuggernyautGrammarLexer ${JUG_ANTLR4_LEXER_PATH} LEXER
            PACKAGE GeneratedLexer)
antlr_target(JuggernyautGrammarParser ${JUG_ANTLR4_PARSER_PATH} PARSER
            PACKAGE GeneratedParser
            DEPENDS_ANTLR JuggernyautGrammarLexer
            COMPILE_FLAGS -lib ${ANTLR_JuggernyautGrammarLexer_OUTPUT_DIR})

# include generated files in project environment
include_directories(${ANTLR_JuggernyautGrammarLexer_OUTPUT_DIR})
include_directories(${ANTLR_JuggernyautGrammarParser_OUTPUT_DIR})
