message(STATUS "[DEPENDENCIES] Checking dependencies...")

include(FetchContent)

# googletest
set(GTEST_VERSION 1.17.0)
set(INSTALL_GTEST OFF CACHE BOOL "Disable GTest installation" FORCE)
if((DEFINED GTEST_VERSION) AND (NOT TARGET googletest))
    message(STATUS "[DEPENDENCIES] Fetching googletest...")
    set(JUG_DEP_GTEST_LIB_PATH ${JUG_DEPENDENCIES_DIR}/gtest)
    # Prevent compiler settings override
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    if(EXISTS ${JUG_DEP_GTEST_LIB_PATH}/CMakeLists.txt)
        FetchContent_Declare(
            googletest
            SOURCE_DIR ${JUG_DEP_GTEST_LIB_PATH}
            EXCLUDE_FROM_ALL
            SYSTEM
        )
    else()
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v${GTEST_VERSION}
            SOURCE_DIR ${JUG_DEP_GTEST_LIB_PATH}
            EXCLUDE_FROM_ALL
            SYSTEM
        )
    endif()
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
    # Ignore known warnings
    check_cxx_compiler_flag("-Wcharacter-conversion" HAS_CHAR_CONV_FLAG)
    if(HAS_CHAR_CONV_FLAG)
        if(TARGET gtest)
            target_compile_options(gtest PRIVATE -Wno-character-conversion)
        endif()
        if(TARGET gtest_main)
            target_compile_options(gtest_main PRIVATE -Wno-character-conversion)
        endif()
    endif()
endif()
