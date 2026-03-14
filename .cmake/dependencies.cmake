message(STATUS "[DEPENDENCIES] Checking dependencies...")

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
