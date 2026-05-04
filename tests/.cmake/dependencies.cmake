message(STATUS "[DEPENDENCIES] Checking dependencies...")

include(FetchContent)

# googletest
set(GTEST_VERSION 1.17.0)
set(INSTALL_GTEST OFF CACHE BOOL "Disable GTest installation" FORCE)
if((DEFINED GTEST_VERSION) AND (NOT TARGET googletest))
    message(STATUS "[DEPENDENCIES] Fetching googletest...")
    set(JUG_DEP_GTEST_LIB_PATH ${JUG_DEPENDENCIES_DIR}/gtest)
    if(EXISTS ${JUG_DEP_GTEST_LIB_PATH}/CMakeLists.txt)
        FetchContent_Declare(
            googletest
            GIT_TAG v${GTEST_VERSION}
            SOURCE_DIR ${JUG_DEP_GTEST_LIB_PATH}
        )
    else()
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v${GTEST_VERSION}
            SOURCE_DIR ${JUG_DEP_GTEST_LIB_PATH}
        )
    endif()
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
endif()
