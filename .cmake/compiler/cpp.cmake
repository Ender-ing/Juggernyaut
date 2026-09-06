# Check for C++20
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Recommended for modern C++

# Make symbols hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)

# Fix ANTLR4-related glitch on Windows
if (MSVC)
    add_compile_definitions(NOMINMAX)
endif()

# Force PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# IPO
cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)

# Enable IPO
check_ipo_supported(RESULT ipo_supported OUTPUT error)
if(ipo_supported AND (NOT CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif((NOT ipo_supported) AND (NOT CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message(WARNING "IPO/LTO is not supported: ${error}")
endif()

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
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
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
macro(add_c_cpp_global_flag flag)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flag}")
    add_compile_options(${flag})
    add_link_options(${flag})
endmacro()

# Make adding global compile definitions easier
function(add_global_compile_definition definition)
    get_property(targets GLOBAL PROPERTY ALL_TARGETS)
    foreach(target ${targets})
        if(TARGET ${target})
            target_compile_definitions(${target} PRIVATE ${definition})
        endif()
    endforeach()
endfunction()

include(CheckCompilerFlag)
macro(check_cxx_flags_support FLAGS RESULT_VAR)
    # Save the original state of required flags
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(OLD_CMAKE_REQUIRED_LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})

    # Forward the flags to BOTH the compiler test and the linker test
    set(CMAKE_REQUIRED_FLAGS "${OLD_CMAKE_REQUIRED_FLAGS} ${FLAGS}")
    set(CMAKE_REQUIRED_LINK_OPTIONS "${OLD_CMAKE_REQUIRED_LINK_OPTIONS}" "${FLAGS}")

    # Check CXX (C++) capability. (Change to 'C' if checking a pure C project)
    check_compiler_flag(CXX "${FLAGS}" ${RESULT_VAR})

    # Restore flags
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_LINK_OPTIONS ${OLD_CMAKE_REQUIRED_LINK_OPTIONS})
endmacro()

# Address Sanitizer
check_cxx_flags_support("-fsanitize=address" HAVE_ASAN)
# Undefined Behavior Sanitizer
check_cxx_flags_support("-fsanitize=undefined" HAVE_UBSAN)
# Combined Address & Undefined
check_cxx_flags_support("-fsanitize=address,undefined" HAVE_ASAN_UBSAN)
# Thread Sanitizer (Note: cannot be combined with ASan!)
check_cxx_flags_support("-fsanitize=thread" HAVE_TSAN)

if(MSVC)
    add_compile_options(
        /external:W0            # Warning level 0 for external headers
        #/external:anglebrackets # Treat all < > includes as external # NOOOOOO!
        /analyze:external-      # Disable static analysis for external headers!
    )
endif()
macro(ignore_external_target_warnings TARGET)
    if(MSVC)
        # /w disables all compiler warnings
        target_compile_options(${TARGET} PRIVATE /w)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # -w inhibits all warning messages
        target_compile_options(${TARGET} PRIVATE -w)
    endif()
    # Supress all warnings
    set_target_properties(${TARGET} PROPERTIES SYSTEM TRUE)
endmacro()

# Include the module to check compiler flags
include(CheckCXXCompilerFlag)

# Ignore GCC version-related warnings
check_cxx_compiler_flag("-Wno-psabi" HAS_WNO_PSABI)
if(HAS_WNO_PSABI)
    add_compile_options(-Wno-psabi)
endif()

# Force symbols generation
if(MSVC)
    add_compile_options(/Zi)
    add_link_options(/DEBUG)
else()
    add_compile_options(-g) # May embed debug symbols into the binary

    # Check for built-in split flags
    check_cxx_compiler_flag("-gsplit-dwarf" HAS_GSPLIT_DWARF)
    if(HAS_GSPLIT_DWARF)
        add_compile_options(-gsplit-dwarf)
    endif()
endif()
