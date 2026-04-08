# Add executable and include relevant files
message(STATUS "[BUILD] Adding executable target 'JuggernyautCompiler'...")
add_executable(
    JuggernyautCompiler ${JUG_COMPILER_MAIN_CPP_PATH}
)

# Link core libraries
add_dependencies(JuggernyautCompiler JuggernyautSessionLibrary)
target_link_libraries(JuggernyautCompiler PUBLIC JuggernyautSessionLibrary)

# Link compiler libraries
include(${JUG_COMPILER_CMAKE_DIR}/libraries.cmake)
foreach(LIB ${COMPILER_LIBRARIES})
    message(STATUS "[BUILD] Adding linking target '${LIB}' to executable target 'JuggernyautCompiler'...")
    # Mark the library as a dependency of the executable
    add_dependencies(JuggernyautCompiler ${LIB})
    # Add the library
    target_link_libraries(JuggernyautCompiler PRIVATE ${LIB})
endforeach()

# Handle dynamic libraries
target_link_directories(JuggernyautCompiler PRIVATE "$<TARGET_FILE_DIR:JuggernyautCompiler>")

# Attach manifest data
# The first use of the "attach_manifest_data" function must be for the main executable!
attach_manifest_data(JuggernyautCompiler ${JUG_COMPILER_MANIFEST_FILE} TRUE)

# Re-do symbolic linking (POST BUILD)
manage_symbolic_links(JuggernyautCompiler "jug")

# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCompiler FALSE)

# Add debug flags
include(${JUG_CMAKE_DIR}/debug-mode.cmake)
