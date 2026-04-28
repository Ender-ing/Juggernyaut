message(STATUS "[DEPENDENCIES] Checking dependencies...")

include(FetchContent)

# toml++
if(NOT DEFINED TOMLPP_INCLUDE_DIR)
    set(TOMLPP_VERSION 3.4.0 CACHE STRING "toml++ version" FORCE)
    set(JUG_DEP_TOMLPP_LIB_PATH ${JUG_DEPENDENCIES_DIR}/tomlpp)
    set(TOMLPP_INCLUDE_DIR "${JUG_DEP_TOMLPP_LIB_PATH}/include" CACHE STRING "toml++ include dir" FORCE)
    if(EXISTS ${JUG_DEP_TOMLPP_LIB_PATH}/CMakeLists.txt)
        FetchContent_Declare(
            tomlplusplus
            GIT_TAG v${TOMLPP_VERSION}
            SOURCE_DIR ${JUG_DEP_TOMLPP_LIB_PATH}
        )
    else()
        FetchContent_Declare(
            tomlplusplus
            GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
            GIT_TAG v${TOMLPP_VERSION}
            SOURCE_DIR ${JUG_DEP_TOMLPP_LIB_PATH}
        )
    endif()
    FetchContent_MakeAvailable(tomlplusplus)
endif()
