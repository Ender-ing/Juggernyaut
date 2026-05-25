message(STATUS "[DEPENDENCIES] Checking dependencies...")

include(FetchContent)

# libgit2
set(GIT2_LIB_VERSION v1.9.3)
if(libgit2)
    message(STATUS "[DEPENDENCIES] libgit2 library is present!")
else()
    # Build dynamic libs
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    # Prevent tests & tools from being built
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_CLI OFF CACHE BOOL "" FORCE)
    # Manage configs
    set(USE_NSEC OFF CACHE BOOL "" FORCE)
    set(USE_XDIFF ON CACHE BOOL "" FORCE)
    # Download libgit2
    message(STATUS "[DEPENDENCIES] Fetching libgit2...")
    set(JUG_DEP_GIT2_LIB_PATH ${JUG_DEPENDENCIES_DIR}/libgit2)
    if(EXISTS ${JUG_DEP_GIT2_LIB_PATH}/CMakeLists.txt)
        FetchContent_Declare(libgit2
            GIT_TAG ${GIT2_LIB_VERSION}
            SOURCE_DIR ${JUG_DEP_GIT2_LIB_PATH})
    else()
        FetchContent_Declare(libgit2
            GIT_REPOSITORY https://github.com/libgit2/libgit2.git
            GIT_TAG ${GIT2_LIB_VERSION}
            SOURCE_DIR ${JUG_DEP_GIT2_LIB_PATH})
    endif()
    FetchContent_MakeAvailable(libgit2)

    if(TARGET libgit2package)
        ignore_external_target_warnings(libgit2package)
    endif()
endif()
