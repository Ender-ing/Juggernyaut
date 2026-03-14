# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

# Create a library from / (the root directory of /compiler)
add_library(JuggernyautBaseLibrary SHARED)
target_sources_search(JuggernyautBaseLibrary ${JUG_CORE_SOURCE_DIR}/base.*.cpp FALSE)
# Expose library exports
target_compile_definitions(JuggernyautBaseLibrary PRIVATE JUG_BASE_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautBaseLibrary ${JUG_CORE_MANIFEST_FILE})
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautBaseLibrary FALSE)
# Link other libraries to the library
add_dependencies(JuggernyautBaseLibrary JuggernyautCommonLibrary)
target_link_libraries(JuggernyautBaseLibrary PUBLIC JuggernyautCommonLibrary)
add_dependencies(JuggernyautBaseLibrary JuggernyautCommsLibrary)
target_link_libraries(JuggernyautBaseLibrary PUBLIC JuggernyautCommsLibrary)

# Create a library from /comms
add_library(JuggernyautCommsLibrary SHARED)
target_sources_search(JuggernyautCommsLibrary ${JUG_CORE_SOURCE_DIR}/comms/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautCommsLibrary PRIVATE JUG_COMMS_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautCommsLibrary ${JUG_CORE_MANIFEST_FILE})
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCommsLibrary FALSE)
# Link other libraries to the library
add_dependencies(JuggernyautCommsLibrary fmt::fmt)
target_link_libraries(JuggernyautCommsLibrary PUBLIC fmt::fmt)

# Create a library from /common
add_library(JuggernyautCommonLibrary SHARED)
target_sources_search(JuggernyautCommonLibrary ${JUG_CORE_SOURCE_DIR}/common/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautCommonLibrary PRIVATE JUG_COMMON_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautCommonLibrary ${JUG_CORE_MANIFEST_FILE})
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCommonLibrary FALSE)

# Create a library from /parser
add_library(JuggernyautParserLibrary SHARED
    ${ANTLR_JuggernyautGrammarLexer_CXX_OUTPUTS} # ANTLR4
    ${ANTLR_JuggernyautGrammarParser_CXX_OUTPUTS} # ANTLR4
)
target_sources_search(JuggernyautParserLibrary ${JUG_CORE_SOURCE_DIR}/parser/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautParserLibrary PRIVATE JUG_PARSER_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautParserLibrary ${JUG_CORE_MANIFEST_FILE})
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautParserLibrary TRUE)
# ANTLR4
add_dependencies(JuggernyautParserLibrary antlr4_shared)
target_link_libraries(JuggernyautParserLibrary PUBLIC antlr4_shared)
# Libraries
if(WIN32)
    set(ANTLR4_DYNAMIC_LIBRARY_COPY_NAME ${ANTLR4_RUNTIME_LIBRARIES})
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(ANTLR4_DYNAMIC_LIBRARY_COPY_NAME ${ANTLR4_OUTPUT_DIR}/*.${ANTLR4_TAG}.dylib)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(ANTLR4_DYNAMIC_LIBRARY_COPY_NAME ${ANTLR4_OUTPUT_DIR}/*.so.${ANTLR4_TAG})
endif()
add_custom_command(TARGET JuggernyautParserLibrary
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND}
                           -E copy ${ANTLR4_DYNAMIC_LIBRARY_COPY_NAME} .
                    WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
# Link other libraries to the library
add_dependencies(JuggernyautParserLibrary JuggernyautCommsLibrary) # TMP
target_link_libraries(JuggernyautParserLibrary PUBLIC JuggernyautCommsLibrary) # TMP

# Expose the core libraries
set(CORE_LIBRARIES
    JuggernyautBaseLibrary
    JuggernyautCommonLibrary
    JuggernyautCommsLibrary
    JuggernyautParserLibrary
    PARENT_SCOPE
)
