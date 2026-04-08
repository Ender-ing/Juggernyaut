# Add debug-specific configurations to all targets
if(${JUG_BINARY_MODE} STREQUAL "Debug")
    message(STATUS "[BUILD] Adding debug compilation flags...")
    # (Windows) CrtDebug
    if(WIN32)
        #add_global_compile_definition("JUG_WINDOWS_CRTDEBUG")
    # (Unix-like) Clang AddressSanitizer
    elseif(CMAKE_CXX_SUPPORTS_FSANITIZE_ADDRESS)
        add_c_cpp_global_flag("-fsanitize=address")
    else()
        message(FATAL_ERROR "[BUILD] Current setup does not allow for debug binaries generation.")
    endif()
endif()
