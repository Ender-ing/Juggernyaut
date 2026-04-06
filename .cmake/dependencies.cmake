message(STATUS "[DEPENDENCIES] Checking dependencies...")

include(ExternalProject)

# VERSION CONTROL
# Manage the versions for used dependencies

# Powershell (on Windows)
if(WIN32)
    find_program(POWERSHELL_EXECUTABLE powershell)
    if(POWERSHELL_EXECUTABLE)
        message(STATUS "[DEPENDENCIES] PowerShell found: ${POWERSHELL_EXECUTABLE}")
    else()
        message(FATAL_ERROR "[DEPENDENCIES] PowerShell not found.")
    endif()
endif()

if(DEFINED NEEDS_WIN_RC)
    # Windows kits folder
    if(WIN32)
        # Get the latest kits path
        execute_process(
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass -Command "Write-Host (Get-ChildItem 'C:\\Program Files (x86)\\Windows Kits\\10\\bin\\' | Sort-Object {$_.VersionInfo.FileVersion} -Descending | Where-Object { $_.Name -match '^\\d' } | Select-Object -First 1).FullName"
            OUTPUT_VARIABLE WINDOWS_KITS_BIN_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        # Get the system platform
        if(CMAKE_GENERATOR_PLATFORM)
            if(${CMAKE_GENERATOR_PLATFORM} STREQUAL "Win32")
                set(WINDOWS_KITS_PLATFORM "x86")
            elseif(${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
                set(WINDOWS_KITS_PLATFORM "x64")
            elseif(${CMAKE_GENERATOR_PLATFORM} STREQUAL "ARM64")
                set(WINDOWS_KITS_PLATFORM "arm64")
            else()
                message(FATAL_ERROR "[DEPENDENCIES] Windows Kits binaries architecture not supported.")
            endif()
        else()
            message(FATAL_ERROR "[DEPENDENCIES] Couldn't determine the target architecture for the Windows Kits binaries.")
        endif()
        set(WINDOWS_KITS_PATH "${WINDOWS_KITS_BIN_PATH}\\${WINDOWS_KITS_PLATFORM}")

        # Check final path!
        if(WINDOWS_KITS_PATH)
            message(STATUS "[DEPENDENCIES] Windows Kits bin path: '${WINDOWS_KITS_PATH}'")
        else()
            message(FATAL_ERROR "[DEPENDENCIES] Could not find Windows Kits bin folder.")
        endif()
    endif()

    # RC command (on Windows)
    if(WIN32)
        find_program(RC_EXECUTABLE rc.exe)
        if(RC_EXECUTABLE)
            message(STATUS "[DEPENDENCIES] rc.exe found: ${RC_EXECUTABLE}")
        else()
            find_program(RC_EXECUTABLE ${WINDOWS_KITS_PATH}/rc.exe)
            if(RC_EXECUTABLE)
                message(STATUS "[DEPENDENCIES] rc.exe found: ${RC_EXECUTABLE} (PATH not set!)")
            else()
                message(FATAL_ERROR "[DEPENDENCIES] rc.exe not found. Windows resource compilation will not work.")
            endif()
        endif()
    endif()
endif()

# Check for GIT Patch
find_program(GIT_PATCH_EXECUTABLE NAMES patch) # Search for "patch" (case-insensitive)
if(GIT_PATCH_EXECUTABLE)
    message(STATUS "[DEPENDENCIES] Git Patch executable found: ${GIT_PATCH_EXECUTABLE}")
else()
    # Attempt to locate the Git Patch executable using the default paths
    if(CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(GIT_PATCH_EXECUTABLE "C:/Program Files/Git/usr/bin/patch.exe")
    else()
        set(GIT_PATCH_EXECUTABLE "/usr/bin/patch")
    endif()
    if(GIT_PATCH_EXECUTABLE)
        message(STATUS "[DEPENDENCIES] Git Patch executable found (default path, not in PATH): ${GIT_PATCH_EXECUTABLE}")
    else()
        message(FATAL_ERROR "[DEPENDENCIES] Git Patch executable not found. Add it to your PATH or install it!")
    endif()
endif()

# Check for Java
if(DEFINED NEED_JAVA)
    find_package(Java REQUIRED)
    # Execute java -version and capture the output
    if(JAVA_FOUND)
        execute_process(
            COMMAND ${Java_JAVA_EXECUTABLE} -version
            OUTPUT_VARIABLE JAVA_VERSION_COMMAND_STDOUT
            ERROR_VARIABLE JAVA_VERSION_COMMAND_STDERR
        )
        # Use the value of either standard output or standard error
        if(JAVA_VERSION_COMMAND_STDOUT)
            set(JAVA_VERSION_COMMAND_OUTPUT ${JAVA_VERSION_COMMAND_STDOUT})
        else()
            set(JAVA_VERSION_COMMAND_OUTPUT ${JAVA_VERSION_COMMAND_STDERR})
        endif()
        # Parse the version string from the find_package output
        string(REGEX MATCH "version \\\"([0-9\\._]+)\\\"" JAVA_VERSION_MATCH "${JAVA_VERSION_COMMAND_OUTPUT}")
        if(JAVA_VERSION_MATCH)
            set(JAVA_VERSION ${CMAKE_MATCH_1})
            # Parse major, minor, patch (example - adapt as needed)
            string(REGEX MATCH "([0-9]+)(\\.([0-9]+))*" JAVA_VERSION_PARTS "${JAVA_VERSION}")
            if(JAVA_VERSION_PARTS)
                set(JAVA_MAJOR_VERSION ${CMAKE_MATCH_1})
            endif()
        else()
            message(FATAL_ERROR "[DEPENDENCIES] Could not determine Java version from find_package output: ${JAVA_VERSION_COMMAND_OUTPUT}")
        endif()
    endif()
    # Check if Java version is 11 or higher
    if(JAVA_MAJOR_VERSION VERSION_GREATER_EQUAL 11) # ANTLR4 jar requires Java 11 or higher 
        message(STATUS "[DEPENDENCIES] Java 11 or higher found: ${JAVA_MAJOR_VERSION}")
    elseif(JAVA_FOUND) # Java was found, but not the right version
        message(FATAL_ERROR "[DEPENDENCIES] Java ${JAVA_MAJOR_VERSION} found, but Java 11+ is required!")
    else()
        message(FATAL_ERROR "[DEPENDENCIES] Java not found! Please install Java 11 or newer on your system.")
    endif()
endif()

# ANTLR4 jar
if(DEFINED ANTLR4_TAG)
    set(JUG_DEPENDENCIES_ANTLR4_JAR_PATH ${JUG_DEPENDENCIES_DIR}/antlr4/antlr.jar)
    if(NOT EXISTS ${JUG_DEPENDENCIES_ANTLR4_JAR_PATH})
        message(STATUS "[DEPENDENCIES] Downloading ANTLR4 jar (v${ANTLR4_TAG})...")
        file(DOWNLOAD
            "https://www.antlr.org/download/antlr-${ANTLR4_TAG}-complete.jar"
            ${JUG_DEPENDENCIES_ANTLR4_JAR_PATH}
            EXPECTED_HASH "SHA256=${ANTLR4_HASH}"
            TLS_VERIFY TRUE
            TIMEOUT 30
            STATUS download_status_1
        )
        if(NOT download_status_1 EQUAL 0)
            message(FATAL_ERROR "[DEPENDENCIES] Failed to download ANTLR4 jar: ${download_errors}")
        endif()
    else()
        message(STATUS "[DEPENDENCIES] ANTLR4 jar is present!")
    endif()
endif()
set(ANTLR_EXECUTABLE ${JUG_DEPENDENCIES_ANTLR4_JAR_PATH})

# mimalloc
if(NOT TARGET mimalloc)
    set(MIMALLOC_VERSION 3.2.8 CACHE STRING "mimalloc version" FORCE)
    FetchContent_Declare(
        mimalloc
        GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
        GIT_TAG v${MIMALLOC_VERSION}
        SOURCE_DIR ${JUG_DEPENDENCIES_DIR}/mimalloc
    )
    set(MI_BUILD_STATIC OFF CACHE BOOL "Build static library" FORCE)
    set(MI_BUILD_SHARED ON CACHE BOOL "Build shared library" FORCE)
    set(MI_BUILD_OBJECT OFF CACHE BOOL "Build object library" FORCE)
    set(MI_BUILD_TESTS OFF CACHE BOOL "Skip mimalloc tests" FORCE)
    set(MI_OVERRIDE ON CACHE BOOL "Override standard allocations" FORCE)
    FetchContent_MakeAvailable(mimalloc)
    set(MIMALLOC_LIB_PATH $<TARGET_FILE:mimalloc> CACHE STRING "mimalloc shared library path" FORCE)
    set(MIMALLOC_WIN_REDIRECT_LIB_PATH "$<TARGET_FILE_DIR:mimalloc>/mimalloc-redirect.dll" CACHE STRING "mimalloc redirect shared library path" FORCE)
endif()
link_libraries(mimalloc)
