message(STATUS "[DEPENDENCIES] Checking dependencies...")

# VERSION CONTROL
# Manage the versions for used dependencies

# {fmt} library
set(FMT_LIB_VERSION 10.2.1)
#Currently, I can't figure out why v11.1.4 doesn't like c++20

# {fmt}
option(USE_INSTALLED_FMT "Ignore or use installed FMT" OFF) # Default OFF - not using installed fmt
if(USE_INSTALLED_FMT)
    find_package(fmt)
endif()
if(fmt)
    message(STATUS "[DEPENDENCIES] {fmt} library is present!")
else()
    set(FMT_INSTALL OFF CACHE BOOL "Disable fmt installation" FORCE)
    # Download {fmt}
    message(STATUS "[DEPENDENCIES] Fetching {fmt}...")
    set(JUG_DEP_FMT_LIB_PATH ${JUG_DEPENDENCIES_DIR}/fmt)
    if(EXISTS ${JUG_DEP_FMT_LIB_PATH}/CMakeLists.txt)
        FetchContent_Declare(fmt
            GIT_TAG ${FMT_LIB_VERSION}
            SOURCE_DIR ${JUG_DEP_FMT_LIB_PATH})
    else()
        FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG ${FMT_LIB_VERSION}
            SOURCE_DIR ${JUG_DEP_FMT_LIB_PATH})
    endif()
    FetchContent_MakeAvailable(fmt)

    # FIX: Ignore known warnings in VS2026
    if(MSVC)
        target_compile_options(fmt INTERFACE /analyze-)
    endif()
endif()
