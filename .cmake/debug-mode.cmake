# Add debug-specific configurations to all targets
if(JUG_BINARY_MODE STREQUAL "Debug")
    message(STATUS "[BUILD] Adding debug compilation flags...")
    # (Windows) CrtDebug
    if(WIN32)
        message(STATUS "[BUILD] Enabling Windows CRT Debug & Runtime Checks.")

        #add_global_compile_definition("JUG_WINDOWS_CRTDEBUG")
        
        # Force CRT Debug Heap memory tracking functions (_malloc_dbg, etc.)
        add_compile_definitions(_DEBUG)
        
        # Generates PDB files and enables basic runtime error checks (stack frames + uninitialized variables)
        add_compile_options(/Zi /RTC1)
        
        # Tell the linker to output debugging information (.pdb)
        add_link_options(/DEBUG)
    elseif(HAVE_ASAN_UBSAN)
        message(STATUS "[BUILD] Enabling ASan + UBSan protections.")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address,undefined)
    elseif(HAVE_ASAN)
        message(STATUS "[BUILD] Enabling ASan protections.")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address)
    else()
        message(FATAL_ERROR "[BUILD] Current setup does not allow for debug binaries generation.")
    endif()
endif()
