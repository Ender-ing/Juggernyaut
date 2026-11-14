message(STATUS "[DEPENDENCIES] Checking dependencies...")

# ANTLR4
set(NEED_JAVA ON)
set(ANTLR4_TAG 4.13.2)
set(ANTLR4_HASH eae2dfa119a64327444672aff63e9ec35a20180dc5b8090b7a6ab85125df4d76)

# toml++
if(NOT DEFINED TOMLPP_INCLUDE_DIR)
    set(TOMLPP_VERSION 3.4.0 CACHE STRING "toml++ version" FORCE)
    set(JUG_DEP_TOMLPP_LIB_PATH ${JUG_DEPENDENCIES_DIR}/tomlpp)
    set(TOMLPP_INCLUDE_DIR "${JUG_DEP_TOMLPP_LIB_PATH}/include" CACHE STRING "toml++ include dir" FORCE)
    if(EXISTS ${JUG_DEP_TOMLPP_LIB_PATH}/CMakeLists.txt)
        FetchContent_Declare(
            tomlplusplus
            SOURCE_DIR ${JUG_DEP_TOMLPP_LIB_PATH}
            SYSTEM
        )
    else()
        FetchContent_Declare(
            tomlplusplus
            GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
            GIT_TAG v${TOMLPP_VERSION}
            SOURCE_DIR ${JUG_DEP_TOMLPP_LIB_PATH}
            SYSTEM
        )
    endif()
    FetchContent_MakeAvailable(tomlplusplus)
endif()
