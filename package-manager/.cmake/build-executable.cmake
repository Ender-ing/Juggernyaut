# Add executable and include relevant files
message(STATUS "[BUILD] Adding executable target 'JuggernyautPackageManager'...")
add_executable(
    JuggernyautPackageManager ${JUG_PACKAGE_MANAGER_MAIN_CPP_PATH}
)

# Add internal files
#target_sources_search(JuggernyautPackageManager ${JUG_PACKAGE_MANAGER_SOURCE_DIR}/???/*.cpp FALSE)

# Link libraries
jug_common(JuggernyautPackageManager)
# LINK THE TOML READER!

# Link external libraries
# libgit2
add_dependencies(JuggernyautPackageManager libgit2 libgit2package)
target_link_libraries(JuggernyautPackageManager PRIVATE libgit2package)

# Handle dynamic libraries
target_link_directories(JuggernyautPackageManager PRIVATE "$<TARGET_FILE_DIR:JuggernyautPackageManager>")

# Attach manifest data
# The first use of the "attach_manifest_data" function must be for the main executable!
attach_manifest_data(JuggernyautPackageManager ${JUG_PACKAGE_MANAGER_MANIFEST_FILE} TRUE)

# Add compiler flags
add_internal_target_cxx_flags(JuggernyautPackageManager FALSE)

# Add debug flags
include(${JUG_CMAKE_DIR}/debug-mode.cmake)
