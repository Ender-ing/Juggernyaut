include(FetchContent)

set(ANTLR4_GIT_REPOSITORY https://github.com/antlr/antlr4.git)
if(NOT DEFINED ANTLR4_TAG)
  # Set to branch name to keep library updated at the cost of needing to rebuild after 'clean'
  # Set to commit hash to keep the build stable and does not need to rebuild after 'clean'
  set(ANTLR4_TAG master)
endif()

if (DEFINED JUG_DEPENDENCIES_DIR)
    set(JUG_DEP_ANTLR4_LIB_PATH "${JUG_DEPENDENCIES_DIR}/antlr4")
else()
    set(JUG_DEP_ANTLR4_LIB_PATH "${CMAKE_CURRENT_BINARY_DIR}/antlr4")
endif()

set(ANTLR4_INSTALL OFF CACHE BOOL "" FORCE)
set(WITH_STATIC_CRT ${ANTLR4_WITH_STATIC_CRT} CACHE BOOL "" FORCE)
set(DISABLE_WARNINGS ON CACHE BOOL "" FORCE)

set(BUILD_TESTING OFF CACHE BOOL "Disable tests globally for dependencies" FORCE)
set(WITH_DEMO OFF CACHE BOOL "Disable ANTLR4 demo" FORCE)
set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "Disable ANTLR4 C++ tests" FORCE)

if (NOT TARGET antlr4_shared)
    if(EXISTS ${JUG_DEP_ANTLR4_LIB_PATH}/runtime/Cpp/CMakeLists.txt)
        FetchContent_Declare(
            antlr4_runtime
            SOURCE_DIR ${JUG_DEP_ANTLR4_LIB_PATH}
            SOURCE_SUBDIR  runtime/Cpp
            SYSTEM
        )
    else()
        FetchContent_Declare(
            antlr4_runtime
            GIT_REPOSITORY ${ANTLR4_GIT_REPOSITORY}
            GIT_TAG        ${ANTLR4_TAG}

            SOURCE_DIR ${JUG_DEP_ANTLR4_LIB_PATH}
            SOURCE_SUBDIR  runtime/Cpp
            SYSTEM

            PATCH_COMMAND "${CMAKE_COMMAND}" 
                "-DPATCH_DIR=${JUG_CMAKE_DIR}/ANTLR4/patches"
                "-DPATCH_EXE=${GIT_PATCH_EXECUTABLE}"
                -P "${JUG_CMAKE_DIR}/ANTLR4/patches/apply.cmake"
        )
    endif()
    FetchContent_MakeAvailable(antlr4_runtime)
endif()

# Protect ANTLR4's shared target from Juggernyaut's global Windows/Optimization flags
if(TARGET antlr4_static)
    ignore_external_target_warnings(antlr4_static)
    custom_malloc(antlr4_static)
endif()
if(TARGET antlr4_shared)
    ignore_external_target_warnings(antlr4_shared)
    custom_malloc(antlr4_shared)
    # Fix linking issues
    get_target_property(_antlr_defs antlr4_shared INTERFACE_COMPILE_DEFINITIONS)
    if(_antlr_defs)
        # Remove the offending macro from the inherited list
        list(REMOVE_ITEM _antlr_defs "ANTLR4CPP_EXPORTS")
        # Apply the cleaned list back to the target
        set_target_properties(antlr4_shared PROPERTIES INTERFACE_COMPILE_DEFINITIONS "${_antlr_defs}")
    endif()
endif()
