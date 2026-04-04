# Add executable and include relevant files
message(STATUS "[BUILD] Adding executable target 'JuggernyautServer'...")
add_executable(
    JuggernyautServer ${JUG_SERVER_MAIN_CPP_PATH}
)

# Add internal files
target_sources_search(JuggernyautServer ${JUG_SERVER_SOURCE_DIR}/base/*.cpp FALSE)
target_sources_search(JuggernyautServer ${JUG_SERVER_SOURCE_DIR}/store/*.cpp FALSE)
target_sources_search(JuggernyautServer ${JUG_SERVER_SOURCE_DIR}/session/*.cpp FALSE)
target_sources_search(JuggernyautServer ${JUG_SERVER_SOURCE_DIR}/capabilities/*.cpp TRUE)

# Link core libraries
foreach(LIB ${CORE_LIBRARIES})
    message(STATUS "[BUILD] Adding linking target '${LIB}' to executable target 'JuggernyautServer'...")
    # Mark the library as a dependency of the executable
    add_dependencies(JuggernyautServer ${LIB})
    # Add the library
    target_link_libraries(JuggernyautServer PUBLIC ${LIB})
endforeach()

# Link external libraries
# lsp-framework
add_dependencies(JuggernyautServer lsp)
target_link_libraries(JuggernyautServer PUBLIC lsp)

# Handle dynamic libraries
target_link_directories(JuggernyautServer PRIVATE "$<TARGET_FILE_DIR:JuggernyautServer>")

# Attach manifest data
# The first use of the "attach_manifest_data" function must be for the main executable!
attach_manifest_data(JuggernyautServer ${JUG_SERVER_MANIFEST_FILE} TRUE)

# Re-do symbolic linking (POST BUILD)
manage_symbolic_links(JuggernyautServer "jug-lsp")

# Add compiler flags
add_internal_target_cxx_flags(JuggernyautServer FALSE)

# Add debug flags
include(${JUG_CMAKE_DIR}/debug-mode.cmake)
