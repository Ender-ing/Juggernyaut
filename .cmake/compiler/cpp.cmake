# Check for C++20
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Recommended for modern C++

# Make symbols hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Fix ANTLR4-related glitch on Windows
if (MSVC)
    add_compile_definitions(NOMINMAX)
endif()

# Enable $ORIGIN-based RPATH globally
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Force PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set character set flags based on compiler (Must be set to UTF-8)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # GCC or Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finput-charset=UTF-8 -fexec-charset=UTF-8")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
else()
    # Other compilers (optional)
    message(FATAL_ERROR "[C++] Compiler ${CMAKE_CXX_COMPILER_ID} not recognized. Character set flags not set.")
endif()

# Resolve <format> import issues
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
        message(FATAL_ERROR "[C++] GCC version 13.0 or higher is required for <format> support. Detected: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
endif()

# Optimisation flags for Release builds
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # GCC or Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /DNDEBUG")
else()
    # Other compilers (optional)
    message(FATAL_ERROR "[C++] Compiler ${CMAKE_CXX_COMPILER_ID} not recognized. Optimisation flags not set.")
endif()

# Warning flags (only for internal targets)
function(add_internal_target_cxx_flags TARGET IS_LESS_RESTRICTIVE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # GCC or Clang
        target_compile_options(${TARGET} PRIVATE
            -Wall # Enable all warnings

            -Wpointer-arith
            -Wcast-align
            -Wstrict-overflow=4
            -Wwrite-strings
            -Wcast-qual
            -Wconversion
            -Wunreachable-code

            -Wunused

            -Wno-unused-parameter
        )
        # Non-AppleClang flags
        if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            target_compile_options(${TARGET} PRIVATE
                -Wshadow=local
            )
        endif()
        if(NOT IS_LESS_RESTRICTIVE)
            target_compile_options(${TARGET} PRIVATE
                -Wshadow
            )
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC
        target_compile_options(${TARGET} PRIVATE
            /analyze # Extra code analysis (without warning errors)

            /W4 # Enable all warnings

            /we4456 # declaration of identifier hides previous local declaration
            /we4457 # declaration of member hides previous local declaration
            /we4458 # declaration of identifier hides class member
            /we4459 # declaration of formal parameter shadows global declaration
            /Zc:templateScope # apply templates shadowing checks

            /we4826 # Conversion from type 'type1' to type 'type2' requires reinterpretation of object representation

            /we4230 # nonstandard extension used: string literal used for array initialization
            /we4244 # conversion from 'const char [N]' to 'char *', possible loss of data.

            /we4180 # qualifier applied to function type has no meaning; ignored
            /we4190 # 'identifier' has C-linkage specified, but returns UDT 'type' which is not C-linkage-compatible.

            /we4244 # conversion from 'type1' to 'type2', possible loss of data.
            /we4242 # conversion from 'type1' to 'type2', possible loss of data.

            /we4702 # unreachable code

            /we4101 # unreferenced local variable
            /we4189 # local variable is initialized but not referenced
            /we4505 # Unreferenced local function has been removed

            /wd4100 # unreferenced formal parameter (Disable)

            /we5038 # initialisation out of declaration order
        )
        if(NOT IS_LESS_RESTRICTIVE)
            target_compile_options(${TARGET} PRIVATE
                /we6244 # local declaration of <variable> hides previous declaration at <line> of <file>
                /we6246 # Local declaration of <variable> hides declaration of same name in outer scope    
            )
        endif()
    else()
        # Other compilers (optional)
        message(FATAL_ERROR "[C++] Compiler ${CMAKE_CXX_COMPILER_ID} not recognized. Can't set the proper warning flags.")
    endif()
endfunction()

# Treat compiler warnings as errors
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# Check for VS
# (MSVC)
#find_package(Visual Studio)
#if(MSVC)
#    message(SEND_WARNING "[DEPENDENCIES] What version of Visual Studio version do you use? (Professional/Community - default is Community):")
#    set(BUILD_VS_EDITION "Community" CACHE STRING "Professional" FORCE)
#endif()

# Make adding global C/C++ flags easier
function(add_c_cpp_global_flag flag)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flag}")
endfunction()

# Make adding global compile definitions easier
function(add_global_compile_definition definition)
    get_property(targets GLOBAL PROPERTY ALL_TARGETS)
    foreach(target ${targets})
        if(TARGET ${target})
            target_compile_definitions(${target} PRIVATE ${definition})
        endif()
    endforeach()
endfunction()

include(CheckCCompilerFlag)

# Test for address sanitizer support
set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
check_c_compiler_flag("-fsanitize=address" CMAKE_CXX_SUPPORTS_FSANITIZE_ADDRESS)
# Test for undefined behavior sanitizer support
set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
check_c_compiler_flag("-fsanitize=undefined" CMAKE_CXX_SUPPORTS_FSANITIZE_UNDEFINED)
# Test for thread sanitizer support
set(CMAKE_REQUIRED_FLAGS "-fsanitize=thread")
check_c_compiler_flag("-fsanitize=thread" CMAKE_CXX_SUPPORTS_FSANITIZE_THREAD)
# Test for memory sanitizer support
set(CMAKE_REQUIRED_FLAGS "-fsanitize=memory")
check_c_compiler_flag("-fsanitize=memory" CMAKE_CXX_SUPPORTS_FSANITIZE_MEMORY)
# Test for leak sanitizer support
set(CMAKE_REQUIRED_FLAGS "-fsanitize=leak")
check_c_compiler_flag("-fsanitize=leak" CMAKE_CXX_SUPPORTS_FSANITIZE_LEAK)
# Test for address and undefined sanitizer support combined.
set(CMAKE_REQUIRED_FLAGS "-fsanitize=address,undefined")
check_c_compiler_flag("-fsanitize=address,undefined" CMAKE_CXX_SUPPORTS_FSANITIZE_ADDRESS_UNDEFINED)

# For ARM targets without FPU, use soft-float ABI
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm(v([0-9])+)?-?[a-z]?$" AND NOT APPLE)
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            # 32-bit ARM (e.g. Raspberry Pi 1-3, older boards)
            add_compile_options(-march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=soft) 
        else()
            # 64-bit ARM
            add_compile_options(-march=armv8-a)
        endif()
    endif()
endif()
