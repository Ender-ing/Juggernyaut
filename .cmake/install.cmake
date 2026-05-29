message(STATUS "[INSTALL] Setting up target installs...")

# Include required system libraries
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION bin)
set(CMAKE_INSTALL_SYSTEM_LIBRARY_DESTINATION lib)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT CmpSystemRuntimeLibs)
set(CMAKE_INSTALL_SYSTEM_LIBRARY_COMPONENT CmpSystemRuntimeLibs)

# Explicitly grab Debug and UCRT libraries
if(JUG_BINARY_MODE STREQUAL "Debug")
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif()
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)

# Filter out unknown extra files
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
include(InstallRequiredSystemLibraries)
if (WIN32)
    if(NOT JUG_BINARY_PLATFORM MATCHES "x86_64")
        list(FILTER CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS EXCLUDE REGEX "vcruntime140_1d?\\.dll$")
    endif()
endif()

# Install system libs
install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION}
    COMPONENT ${CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT}
)

# General
install(TARGETS JuggernyautCommonLibrary JuggernyautCatConsoleLibrary
    EXPORT JuggernyautToolchain
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautCompiler
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautCompiler
)

# Core
install(TARGETS JuggernyautDiagnosticsLibrary JuggernyautDataLibrary
        JuggernyautParserLibrary JuggernyautSessionLibrary JuggernyautManagerLibrary
    EXPORT JuggernyautToolchain
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautCompiler
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautCompiler
)

# Configs
install(TARGETS JuggernyautConfigsLibrary
    EXPORT JuggernyautToolchain
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautCompiler
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautCompiler
)

# External deps (without installs/excluded)
if(TARGET mimalloc)
    install(TARGETS mimalloc
        RUNTIME DESTINATION bin COMPONENT CmpJuggernyautCompiler
        LIBRARY DESTINATION lib COMPONENT CmpJuggernyautCompiler
    )
    if(WIN32)
        install(CODE "
            file(GLOB MIMALLOC_REDIRECT_FILES \"${JUG_OUT_FINAL_DIR}/bin/mimalloc-redirect*.dll\")
            
            if(MIMALLOC_REDIRECT_FILES)
                message(STATUS \"[INSTALL] Found runtime redirectors, copying...\")
                file(INSTALL TYPE PROGRAM 
                     FILES \${MIMALLOC_REDIRECT_FILES} 
                     DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin\"
                )
            else()
                message(FATAL_ERROR \"[INSTALL] No mimalloc-redirect DLLs found in ${JUG_OUT_FINAL_DIR}/bin at install time!\")
            endif()
        " COMPONENT CmpJuggernyautCompiler)
    endif()
endif()
install(TARGETS antlr4_shared
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautCompiler
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautCompiler
)
install(TARGETS libgit2package
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautPackageManager
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautPackageManager
)
# Copy ANTLR4 binaries
#add_custom_command(
#    TARGET JuggernyautToolchain
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:antlr4_shared> "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:antlr4_shared> "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# Executables
install(TARGETS JuggernyautCompiler
    EXPORT JuggernyautToolchain
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautCompiler
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautCompiler
)
install(TARGETS JuggernyautServer
    EXPORT JuggernyautToolchain
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautServer
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautServer
)
install(TARGETS JuggernyautPackageManager
    EXPORT JuggernyautToolchain
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautPackageManager
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautPackageManager
)

# Commands
install_command_symlink(JuggernyautCompiler CmpJuggernyautCompiler jug-cmp)
install_command_symlink(JuggernyautServer CmpJuggernyautServer jug-lsp)
install_command_symlink(JuggernyautPackageManager CmpJuggernyautPackageManager jug-pck)

# License & other
install(FILES ${JUG_LICENSE_FILE} ${JUG_CMAKE_DIR}/extra/jug-docs.html
    DESTINATION .
    COMPONENT CmpJuggernyautCompiler
)

# Wrapper
add_executable(jug ${JUG_MAIN_DIR}/wrapper.cpp)
install(TARGETS jug
    EXPORT JuggernyautToolchain
    RUNTIME DESTINATION bin COMPONENT CmpJuggernyautUnified
    LIBRARY DESTINATION lib COMPONENT CmpJuggernyautUnified
)

# Cleanup
if(WIN32)
    set(INSTALL_STATIC_LIB_EXT lib)
else()
    set(INSTALL_STATIC_LIB_EXT a)
endif()
install(CODE "
    message(STATUS \"[POST-INSTALL] Cleaning up extra components...\")
    
    # Dirs
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/include\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/share\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/bin/cmake\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/lib/cmake\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/bin/pkgconfig\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/lib/pkgconfig\")
    
    # Static libs
    file(GLOB_RECURSE static_libs \"\${CMAKE_INSTALL_PREFIX}/*.${INSTALL_STATIC_LIB_EXT}\")
    if(static_libs)
        file(REMOVE \${static_libs})
    endif()
    
    message(STATUS \"[POST-INSTALL] Cleanup complete!\")
" COMPONENT CmpJuggernyautCompiler)

# Verify arch on demand
if(JUG_MATCH_INSTALL_ARCH)
    install(CODE "
        message(STATUS \"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\")
        message(STATUS \"[POST-INSTALL] Starting Binary Architecture Scan...\")
        message(STATUS \"[POST-INSTALL] Target directory: \${CMAKE_INSTALL_PREFIX}\")
        message(STATUS \"[POST-INSTALL] Expected Type: ${JUG_BINARY_PLATFORM}\")
        message(STATUS \"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\")

        set(SCAN_DIRS 
            \"\${CMAKE_INSTALL_PREFIX}/bin\"
            \"\${CMAKE_INSTALL_PREFIX}/lib\"
        )

        set(FOUND_ARTIFACTS \"\")
        foreach(SCAN_DIR IN LISTS SCAN_DIRS)
            if(EXISTS \"\${SCAN_DIR}\")
                # Pulls Windows (.exe, .dll), Linux (no extension, .so), and macOS (.dylib) formats
                file(GLOB_RECURSE MATCHED_FILES 
                    LIST_DIRECTORIES false
                    \"\${SCAN_DIR}/*\"
                )
                list(APPEND FOUND_ARTIFACTS \${MATCHED_FILES})
            endif()
        endforeach()

        # Filter out other expected files
        set(AUDIT_LIST \"\")
        foreach(ITEM IN LISTS FOUND_ARTIFACTS)
            if(NOT ITEM MATCHES \"\\\\.(pdb|idb|ilk|dSYM|map)\$\")
                list(APPEND AUDIT_LIST \${ITEM})
            endif()
        endforeach()

        # Validate arch
        set(ARCH_MISMATCH_DETECTED OFF)
        set(ARCH_MISMATCHES)
        foreach(BINARY_PATH IN LISTS AUDIT_LIST)
            message(STATUS \"[POST-INSTALL] Verifying: \${BINARY_PATH}\")
        
            execute_process(
                COMMAND \"${Python3_EXECUTABLE}\" 
                        \"${JUG_CMAKE_DIR}/external-scripts/arch_match.py\" 
                        \"\${BINARY_PATH}\" 
                        \"${JUG_BINARY_PLATFORM}\"
                RESULT_VARIABLE AUDIT_RESULT
                OUTPUT_VARIABLE AUDIT_OUTPUT
                ERROR_VARIABLE AUDIT_ERROR
            )
        
            # Greedy mismatch identification 
            if(NOT AUDIT_RESULT EQUAL 0)
                message(STATUS \"[POST-INSTALL] Architecture mismatch:\")
                message(STATUS \"\${AUDIT_OUTPUT}\")
                message(STATUS \"\${AUDIT_ERROR}\")
                set(ARCH_MISMATCH_DETECTED ON)
                list(APPEND ARCH_MISMATCHES \"\n\${AUDIT_OUTPUT}\n\${AUDIT_ERROR}\")
            endif()
        endforeach()

        if(ARCH_MISMATCH_DETECTED)
            message(FATAL_ERROR \"[POST-INSTALL] Architecture mismatch discovered!\n\" \${ARCH_MISMATCHES})
        else()
            message(STATUS \"[POST-INSTALL] All installed binaries match the '${JUG_BINARY_PLATFORM}' profile.\")
        endif()
    " COMPONENT CmpJuggernyautCompiler)
endif()
