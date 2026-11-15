message(STATUS "[DEPENDENCIES] Checking dependencies...")

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

# tree-sitter C runtime
set(TREE_SITTER_VERSION 0.24.7)
set(JUG_DEP_TREE_SITTER_PATH ${JUG_DEPENDENCIES_DIR}/tree-sitter)
if(NOT EXISTS ${JUG_DEP_TREE_SITTER_PATH}/lib/src/lib.c)
    FetchContent_Declare(
        tree_sitter_src
        GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
        GIT_TAG v${TREE_SITTER_VERSION}
        GIT_SHALLOW TRUE
        SOURCE_DIR ${JUG_DEP_TREE_SITTER_PATH}
        SOURCE_SUBDIR __none__ # Do not run tree-sitter's root CMakeLists
    )
    FetchContent_MakeAvailable(tree_sitter_src)
endif()

# cpp-tree-sitter header wrapper
set(JUG_DEP_CPP_TREE_SITTER_PATH ${JUG_DEPENDENCIES_DIR}/cpp-tree-sitter)
if(NOT EXISTS ${JUG_DEP_CPP_TREE_SITTER_PATH}/include/cpp-tree-sitter.h)
    FetchContent_Declare(
        cpp_tree_sitter_src
        GIT_REPOSITORY https://github.com/nwjlyons/cpp-tree-sitter.git
        GIT_TAG main
        GIT_SHALLOW TRUE
        SOURCE_DIR ${JUG_DEP_CPP_TREE_SITTER_PATH}
        SOURCE_SUBDIR __none__ # Header-only; skip its CPM.cmake setup
    )
    FetchContent_MakeAvailable(cpp_tree_sitter_src)
endif()
