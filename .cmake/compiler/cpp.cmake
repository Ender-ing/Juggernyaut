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

# Include modules to check compiler and linker flags
include(CheckCXXCompilerFlag)
include(CheckCompilerFlag)
include(CheckLinkerFlag)

# IPO (Interprocedural Optimization / Link-Time Optimization)
cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)

# Enable IPO per-configuration so multi-config generators (e.g. Visual Studio, Ninja Multi-Config)
# don't accidentally enable LTO in Debug builds.
check_ipo_supported(RESULT ipo_supported OUTPUT error)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
else()
    message(WARNING "IPO/LTO is not supported: ${error}")
endif()

# Set character set flags based on compiler (Must be set to UTF-8)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # GCC or Clang (Linux & macOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finput-charset=UTF-8 -fexec-charset=UTF-8")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC (Windows)
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

# Helper macro to append flags to all non-Debug configurations (Release, RelWithDebInfo, MinSizeRel)
# Using per-config variables avoids broken multi-line generator expressions in Visual Studio / MSBuild.
macro(add_non_debug_cxx_flag flag)
    foreach(cfg RELEASE RELWITHDEBINFO MINSIZEREL)
        set(CMAKE_CXX_FLAGS_${cfg} "${CMAKE_CXX_FLAGS_${cfg}} ${flag}")
    endforeach()
endmacro()

macro(add_non_debug_c_cxx_flag flag)
    foreach(cfg RELEASE RELWITHDEBINFO MINSIZEREL)
        set(CMAKE_C_FLAGS_${cfg} "${CMAKE_C_FLAGS_${cfg}} ${flag}")
        set(CMAKE_CXX_FLAGS_${cfg} "${CMAKE_CXX_FLAGS_${cfg}} ${flag}")
    endforeach()
endmacro()

macro(add_non_debug_link_flag flag)
    foreach(cfg RELEASE RELWITHDEBINFO MINSIZEREL)
        set(CMAKE_EXE_LINKER_FLAGS_${cfg} "${CMAKE_EXE_LINKER_FLAGS_${cfg}} ${flag}")
        set(CMAKE_SHARED_LINKER_FLAGS_${cfg} "${CMAKE_SHARED_LINKER_FLAGS_${cfg}} ${flag}")
        set(CMAKE_MODULE_LINKER_FLAGS_${cfg} "${CMAKE_MODULE_LINKER_FLAGS_${cfg}} ${flag}")
    endforeach()
endmacro()

# Optimisation flags for non-Debug builds
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # GCC or Clang (Shared Linux & macOS compile optimizations)
    add_non_debug_c_cxx_flag("-O3")                         # Maximum standard compiler optimizations (inlining, vectorization, loop unrolling) [matches MSVC /O2 /Ot /Oi]
    add_non_debug_c_cxx_flag("-DNDEBUG")                    # Disable standard assert() checks in release code [matches MSVC /DNDEBUG]
    add_non_debug_c_cxx_flag("-ffunction-sections")         # Place each function into its own section for linker dead-code stripping [matches MSVC /Gy]
    add_non_debug_c_cxx_flag("-fdata-sections")             # Place each global/static variable into its own section for linker dead-data stripping [matches MSVC /Gw]
    add_non_debug_c_cxx_flag("-fmerge-all-constants")       # Merge identical constants and read-only arrays across translation units [matches MSVC /GF string/constant pooling]

    # Linux-specific compile optimizations (not supported on macOS Mach-O)
    if(NOT APPLE)
        add_non_debug_c_cxx_flag("-fno-semantic-interposition") # Assume symbols cannot be interposed via LD_PRELOAD; allows inlining even with -fPIC [matches MSVC DLL symbol behavior]
        add_non_debug_c_cxx_flag("-fno-plt")                # Avoid Procedure Linkage Table (PLT) indirection for external function calls
    endif()

    # Parallel LTO for GCC (matches MSVC multi-threaded LTCG)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_non_debug_c_cxx_flag("-flto=auto")              # Parallelize Link-Time Optimization (LTO) across all available CPU cores on GCC
        add_non_debug_link_flag("-flto=auto")               # Use parallel jobs during the GCC LTO link step
    endif()

    # Linker optimizations: macOS (ld64 / ld_prime) vs Linux (ld / gold / lld)
    if(APPLE)
        add_non_debug_link_flag("-Wl,-dead_strip")          # Strip unreachable functions and data sections on macOS [matches Linux --gc-sections & MSVC /OPT:REF]
        check_linker_flag(CXX "-Wl,-deduplicate" HAS_MACOS_DEDUPLICATE)
        if(HAS_MACOS_DEDUPLICATE)
            add_non_debug_link_flag("-Wl,-deduplicate")     # Fold identical functions/templates on macOS [matches Linux --icf=safe & MSVC /OPT:ICF]
        endif()
    else()
        add_non_debug_link_flag("-Wl,--gc-sections")        # Garbage-collect (strip) unused function and data sections on Linux [matches macOS -dead_strip & MSVC /OPT:REF]
        add_non_debug_link_flag("-Wl,-O2")                  # Enable linker-level optimizations (string table tail-merging and hash table optimization)
        add_non_debug_link_flag("-Wl,--as-needed")          # Only link shared libraries that actually satisfy an unresolved symbol
        check_linker_flag(CXX "-Wl,--icf=safe" HAS_LINUX_ICF_SAFE)
        if(HAS_LINUX_ICF_SAFE)
            add_non_debug_link_flag("-Wl,--icf=safe")       # Perform safe Identical Code Folding on Linux linkers that support it (lld/gold) [matches macOS -deduplicate & MSVC /OPT:ICF]
        endif()
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC (Windows) - C & C++ shared flags
    add_non_debug_c_cxx_flag("/O2")                         # Maximize speed (equivalent to /Og /Oi /Ot /Oy /Ob2 /GF /Gy) [matches GCC/Clang -O3]
    add_non_debug_c_cxx_flag("/Oi")                         # Generate intrinsic functions (replaces function calls with inline CPU instructions)
    add_non_debug_c_cxx_flag("/Ot")                         # Favor fast machine code over small machine code
    add_non_debug_c_cxx_flag("/Ob2")                        # Aggressive function inlining (inline any suitable function)
    add_non_debug_c_cxx_flag("/GF")                         # Enable read-only string pooling [matches GCC/Clang -fmerge-all-constants]
    add_non_debug_c_cxx_flag("/Gw")                         # Package global data in individual COMDAT sections for linker stripping [matches GCC/Clang -fdata-sections]
    add_non_debug_c_cxx_flag("/Gy")                         # Package functions in individual COMDAT sections for linker stripping/folding [matches GCC/Clang -ffunction-sections]
    add_non_debug_c_cxx_flag("/Zc:inline")                  # Remove unreferenced inline/internal-linkage functions & data before emitting .obj (shrinks .obj and speeds up linking)
    add_non_debug_c_cxx_flag("/DNDEBUG")                    # Disable standard assert() checks in release code [matches GCC/Clang -DNDEBUG]

    # MSVC (Windows) - C++-only flags (kept out of CMAKE_C_FLAGS so C targets like zlib/libgit2 don't warn)
    add_non_debug_cxx_flag("/Zc:throwingNew")               # Assume operator new throws on failure (ISO C++ standard), skipping redundant null-pointer checks [default in GCC/Clang]

    # MSVC (Windows) - Linker flags
    add_non_debug_link_flag("/OPT:REF")                     # Eliminate unreferenced functions and data (overrides /DEBUG's default /OPT:NOREF) [matches --gc-sections & -dead_strip]
    add_non_debug_link_flag("/OPT:ICF")                     # Perform Identical COMDAT Folding to merge duplicate templates/functions [matches --icf=safe & -deduplicate]
    add_non_debug_link_flag("/INCREMENTAL:NO")              # Disable incremental linking padding and jump thunks for smaller, faster binaries [default on Linux/macOS]
else()
    # Other compilers (optional)
    message(FATAL_ERROR "[C++] Compiler ${CMAKE_CXX_COMPILER_ID} not recognized. Optimisation flags not set.")
endif()

# Warning flags (only for internal targets)
function(add_internal_target_cxx_flags TARGET IS_LESS_RESTRICTIVE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # GCC or Clang (Linux & macOS)
        target_compile_options(${TARGET} PRIVATE
            -Wall                    # Enable standard warnings [matches MSVC /W4]
            -Wextra                  # Enable extra warning checks [matches MSVC /W4]

            -Wpointer-arith          # Warn on arithmetic with void* or function pointers
            -Wcast-align             # Warn when pointer casts increase required alignment
            -Wstrict-overflow=4      # Warn about optimizations that assume signed overflow does not occur
            -Wwrite-strings          # Give string constants the type const char[N] so copying to non-const char* warns [matches MSVC /we4230 & /we4244]
            -Wcast-qual              # Warn whenever a pointer is cast so as to remove a type qualifier (e.g. const)
            -Wconversion             # Warn for implicit conversions that may alter a value [matches MSVC /we4244 & /we4242]
            -Wsign-conversion        # Warn for implicit conversions that may change the sign of an integer [matches MSVC /we4244]
            -Wfloat-conversion       # Warn for implicit conversions that reduce floating-point precision or convert float to int [matches MSVC /we4244 & /we4242]
            -Wignored-qualifiers     # Warn if return type qualifiers (like const) have no meaning [matches MSVC /we4180]
            -Wreorder                # Warn when member initializers are out of declaration order [matches MSVC /we5038]
            -Wunreachable-code       # Warn about code that will never be executed [matches MSVC /we4702]

            -Wunused                 # Enable all -Wunused-* warnings [matches MSVC /we4101, /we4189, /we4505]
            -Wunused-variable        # Warn about unreferenced local variables [matches MSVC /we4101 & /we4189]
            -Wunused-function        # Warn about unreferenced local/static functions [matches MSVC /we4505]

            -Wno-unused-parameter    # Disable unused parameter warning [matches MSVC /wd4100]
            -Wno-missing-field-initializers # Allow C++20 partial designated initializers [matches MSVC /W4]
        )

        # Clang & AppleClang specific equivalents for MSVC warnings
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${TARGET} PRIVATE
                -Wreturn-type-c-linkage # Warn when extern "C" function returns a C++-only UDT [matches MSVC /we4190]
            )
        endif()

        # Shadowing warnings: GCC vs Clang/AppleClang [matches MSVC /we4456, /we4457, /we4458, /we4459, /we6244, /we6246]
        if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            target_compile_options(${TARGET} PRIVATE
                -Wshadow=local       # Warn when a local variable shadows another local variable or parameter [matches MSVC /we4456, /we4457, /we6244]
            )
        else()
            target_compile_options(${TARGET} PRIVATE
                -Wshadow             # Base local shadowing check on AppleClang
            )
        endif()
        if(NOT IS_LESS_RESTRICTIVE)
            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                target_compile_options(${TARGET} PRIVATE
                    -Wshadow-all     # Warn on all shadowing (local, field, global, uncaught) on Clang/AppleClang [matches MSVC /we4456-4459 & /we6244-6246]
                )
            else()
                target_compile_options(${TARGET} PRIVATE
                    -Wshadow         # Warn on all shadowing (local, member, global) on GCC [matches MSVC /we4456-4459 & /we6244-6246]
                )
            endif()
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC (Windows)
        target_compile_options(${TARGET} PRIVATE
            /analyze # Extra code analysis (without warning errors)

            /W4 # Enable all warnings [matches GCC/Clang -Wall -Wextra]

            /we4456 # declaration of identifier hides previous local declaration [matches -Wshadow=local]
            /we4457 # declaration of member hides previous local declaration [matches -Wshadow=local]
            /we4458 # declaration of identifier hides class member [matches -Wshadow / -Wshadow-field]
            /we4459 # declaration of formal parameter shadows global declaration [matches -Wshadow / -Wshadow-all]
            /Zc:templateScope # apply templates shadowing checks

            /we4826 # Conversion from type 'type1' to type 'type2' is sign-extended / requires reinterpretation [matches -Wconversion / -Wsign-conversion]

            /we4230 # nonstandard extension used: string literal used for array initialization [matches -Wwrite-strings]
            /we4244 # conversion from 'const char [N]' to 'char *', possible loss of data.

            /we4180 # qualifier applied to function type has no meaning; ignored [matches -Wignored-qualifiers]
            /we4190 # 'identifier' has C-linkage specified, but returns UDT 'type' which is not C-linkage-compatible. [matches Clang -Wreturn-type-c-linkage]

            /we4244 # conversion from 'type1' to 'type2', possible loss of data. [matches -Wconversion / -Wfloat-conversion]
            /we4242 # conversion from 'type1' to 'type2', possible loss of data. [matches -Wconversion]

            /we4702 # unreachable code [matches -Wunreachable-code]

            /we4101 # unreferenced local variable [matches -Wunused-variable]
            /we4189 # local variable is initialized but not referenced [matches -Wunused-variable]
            /we4505 # Unreferenced local function has been removed [matches -Wunused-function]

            /wd4100 # unreferenced formal parameter (Disable) [matches -Wno-unused-parameter]

            /we5038 # initialisation out of declaration order [matches -Wreorder]
        )
        if(NOT IS_LESS_RESTRICTIVE)
            target_compile_options(${TARGET} PRIVATE
                /we6244 # local declaration of <variable> hides previous declaration at <line> of <file> [matches -Wshadow / -Wshadow-all]
                /we6246 # Local declaration of <variable> hides declaration of same name in outer scope [matches -Wshadow=local / -Wshadow-all]
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
        /external:W0            # Warning level 0 for external headers (matches GCC/Clang -isystem behavior)
        #/external:anglebrackets # Treat all < > includes as external # NOOOOOO!
        /analyze:external-      # Disable static analysis for external headers!
    )
endif()
macro(ignore_external_target_warnings TARGET)
    get_target_property(_target_type ${TARGET} TYPE)
    if(NOT _target_type STREQUAL "INTERFACE_LIBRARY" AND NOT _target_type STREQUAL "UTILITY")
        if(MSVC)
            # /w disables all compiler warnings
            target_compile_options(${TARGET} PRIVATE /w)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # -w inhibits all warning messages
            target_compile_options(${TARGET} PRIVATE -w)
        endif()
        # Suppress all warnings (passes -isystem on GCC/Clang/AppleClang and /external:I on MSVC)
        set_target_properties(${TARGET} PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
        # Supress all warnings (passes -isystem on GCC/Clang/AppleClang and /external:I on MSVC)
        set_target_properties(${TARGET} PROPERTIES SYSTEM TRUE)
    endif()
endmacro()
function(ignore_external_directory_warnings DIR)
    get_property(DIR_TARGETS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(TARGET IN LISTS DIR_TARGETS)
        ignore_external_target_warnings(${TARGET})
    endforeach()
    get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
    foreach(SUBDIR IN LISTS SUBDIRS)
        ignore_external_directory_warnings("${SUBDIR}")
    endforeach()
endfunction()

# Ignore GCC version-related warnings
check_cxx_compiler_flag("-Wno-psabi" HAS_WNO_PSABI)
if(HAS_WNO_PSABI)
    add_compile_options(-Wno-psabi) # Suppress ABI notes about parameter passing changes across GCC versions
endif()

# Force symbols generation
if(MSVC)
    add_compile_options(/Zi)  # Generate complete debugging information in a separate PDB file [matches -g + -gsplit-dwarf]
    add_link_options(/DEBUG)  # Emit PDB file at link time (note: /OPT:REF, /OPT:ICF, and /INCREMENTAL:NO above prevent /DEBUG from de-optimizing Release builds)
else()
    add_compile_options(-g) # Generate debug information [matches MSVC /Zi]

    # Check for built-in split flags (supported on Linux ELF; skipped automatically on macOS where dSYM / OSO tables are used instead)
    if(NOT APPLE)
        check_cxx_compiler_flag("-gsplit-dwarf" HAS_GSPLIT_DWARF)
        if(HAS_GSPLIT_DWARF)
            add_compile_options(-gsplit-dwarf) # Split DWARF debug info into separate .dwo files to speed up linking [matches MSVC separate PDB behavior]
        endif()
    endif()
endif()

# Preprocessor flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor") # Enable standard-conforming C++20/23 preprocessor (already default on GCC/Clang/AppleClang)
endif()
