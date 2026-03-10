# Add executable and include relevant files
message(STATUS "[BUILD] Adding executable target 'JuggernyautCompiler'...")
add_executable(
    JuggernyautCompiler ${JUG_MAIN_CPP_PATH}
)

# Handle dynamic libraries
target_link_directories(JuggernyautCompiler PRIVATE "$<TARGET_FILE_DIR:JuggernyautCompiler>")

# Attach manifest data
# The first use of the "attach_manifest_data" function must be for the main executable!
attach_manifest_data(JuggernyautCompiler)

# Re-do symbolic linking (POST BUILD)
manage_symbolic_links(JuggernyautCompiler "jug")

# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCompiler FALSE)

# Generate ANTLR4 grammar C++ files
#

# Include project libraries
include(${JUG_CMAKE_DIR}/libraries.cmake)

# Link C++ libraries
# Basic in-house libraries
foreach(LIB ${PROJECT_LIBRARIES})
    message(STATUS "[BUILD] Adding linking target '${LIB}' to executable target 'JuggernyautCompiler'...")
    # Mark the library as a dependency of the executable
    add_dependencies(JuggernyautCompiler ${LIB})
    # Add the library
    target_link_libraries(JuggernyautCompiler PUBLIC ${LIB})
endforeach()

# Add debug flags
include(${JUG_CMAKE_DIR}/debug-mode.cmake)
