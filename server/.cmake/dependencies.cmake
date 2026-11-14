message(STATUS "[DEPENDENCIES] Checking dependencies...")

include(FetchContent)

# lsp-framework
set(LSP_FRAMEWORK_LIB_VERSION 2.0.0)
option(USE_INSTALLED_LSP_FRAMEWORK "Ignore or use installed lsp-framework" OFF) # Default OFF
if(USE_INSTALLED_LSP_FRAMEWORK)
    find_package(lsp ${LSP_FRAMEWORK_LIB_VERSION} EXACT REQUIRED)
endif()
if(lsp)
    message(STATUS "[DEPENDENCIES] lsp-framework library is present!")
else()
    # Download lsp-framework
    message(STATUS "[DEPENDENCIES] Fetching lsp-framework...")
    set(JUG_DEP_LSP_FRAMEWORK_LIB_PATH ${JUG_DEPENDENCIES_DIR}/lsp-framework)
    if(EXISTS ${JUG_DEP_LSP_FRAMEWORK_LIB_PATH}/CMakeLists.txt)
        FetchContent_Declare(lsp
            SOURCE_DIR ${JUG_DEP_LSP_FRAMEWORK_LIB_PATH}
            SYSTEM)
    else()
        FetchContent_Declare(lsp
            GIT_REPOSITORY https://github.com/leon-bckl/lsp-framework.git
            GIT_TAG ${LSP_FRAMEWORK_LIB_VERSION}
            GIT_SUBMODULES "" # Fix submodule fetch issues
            SOURCE_DIR ${JUG_DEP_LSP_FRAMEWORK_LIB_PATH}
            SYSTEM

            PATCH_COMMAND "${CMAKE_COMMAND}" 
                "-DPATCH_DIR=${JUG_CMAKE_DIR}/LSP/patches"
                "-DPATCH_EXE=${GIT_PATCH_EXECUTABLE}"
                -P "${JUG_CMAKE_DIR}/LSP/patches/apply.cmake"
            )
    endif()
    FetchContent_MakeAvailable(lsp)
    ignore_external_directory_warnings("${lsp_BINARY_DIR}")
endif()
if(TARGET lsp)
    # Use custom malloc
    custom_malloc(lsp)
endif()
