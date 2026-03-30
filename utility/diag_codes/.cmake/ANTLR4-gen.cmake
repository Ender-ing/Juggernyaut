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
if(NOT TARGET antlr4_runtime)
    include(ExternalAntlr4Cpp)
endif()
# Push to Global Cache
set(ANTLR4_INCLUDE_DIRS ${ANTLR4_INCLUDE_DIRS} CACHE INTERNAL "ANTLR4 Include Dirs")
set(ANTLR4_RUNTIME_LIBRARIES ${ANTLR4_RUNTIME_LIBRARIES} CACHE INTERNAL "ANTLR4 Libraries")
# add antlr4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

# Call macro to add lexer and grammar to your build dependencies.
antlr_target(DiagnosticsCodeLexer ${JUG_UTILITY_DIAGCODES_ANTLR4_LEXER_PATH} LEXER
            PACKAGE GeneratedLexer)
antlr_target(DiagnosticsCodeParser ${JUG_UTILITY_DIAGCODES_ANTLR4_PARSER_PATH} PARSER
            VISITOR
            PACKAGE GeneratedParser
            DEPENDS_ANTLR DiagnosticsCodeLexer
            COMPILE_FLAGS -lib ${ANTLR_DiagnosticsCodeLexer_OUTPUT_DIR})

# include generated files in project environment
include_directories(${ANTLR_DiagnosticsCodeLexer_OUTPUT_DIR})
include_directories(${ANTLR_DiagnosticsCodeParser_OUTPUT_DIR})
