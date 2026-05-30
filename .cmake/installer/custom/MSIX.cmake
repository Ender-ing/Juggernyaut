cmake_minimum_required(VERSION 4.2 FATAL_ERROR)

####################################################
## USE
####################################################

# list(APPEND CPACK_GENERATOR "External")
# set(CPACK_EXTERNAL_PACKAGE_SCRIPT ".../MSIX.cmake")
# set(CPACK_EXTERNAL_ENABLE_STAGING ON)

####################################################
## SUPPORTED VARIABLES
####################################################

# [VERSION VARIABLES]:
# CPACK_MSIX_PACKAGE_VERSION_MAJOR
if(NOT DEFINED CPACK_MSIX_PACKAGE_VERSION_MAJOR)
    set(CPACK_MSIX_PACKAGE_VERSION_MAJOR 0)
endif()
# CPACK_MSIX_PACKAGE_VERSION_MINOR
if(NOT DEFINED CPACK_MSIX_PACKAGE_VERSION_MINOR)
    set(CPACK_MSIX_PACKAGE_VERSION_MINOR 0)
endif()
# CPACK_MSIX_PACKAGE_VERSION_PATCH
if(NOT DEFINED CPACK_MSIX_PACKAGE_VERSION_PATCH)
    set(CPACK_MSIX_PACKAGE_VERSION_PATCH 0)
endif()
# CPACK_MSIX_PACKAGE_VERSION_REVISION
if(NOT DEFINED CPACK_MSIX_PACKAGE_VERSION_REVISION)
    set(CPACK_MSIX_PACKAGE_VERSION_REVISION 0)
endif()

# [PLATFORM VARIABLES]
# CPACK_MSIX_PACKAGE_ARCHITECTURE (REQUIRED)
if(DEFINED CPACK_MSIX_PACKAGE_ARCHITECTURE)
    # Port over arch names
    if(CPACK_MSIX_PACKAGE_ARCHITECTURE STREQUAL "x86_64")
        set(CPACK_MSIX_PACKAGE_ARCHITECTURE "x64")
    elseif(CPACK_MSIX_PACKAGE_ARCHITECTURE STREQUAL "arm64")
        set(CPACK_MSIX_PACKAGE_ARCHITECTURE "arm64")
    elseif(CPACK_MSIX_PACKAGE_ARCHITECTURE STREQUAL "x86_32")
        set(CPACK_MSIX_PACKAGE_ARCHITECTURE "x86")
    endif()

    if(NOT CPACK_MSIX_PACKAGE_ARCHITECTURE MATCHES "^(x86|x64|arm64)$")
        message(FATAL_ERROR "[CPACK MSIX] Expecting a valid 'CPACK_MSIX_PACKAGE_ARCHITECTURE' value: x86|x64|arm64")
    endif()
else()
    message(FATAL_ERROR "[CPACK MSIX] 'CPACK_MSIX_PACKAGE_ARCHITECTURE' must be set!")
endif()

# [PACKAGE DETAILS]
# CPACK_MSIX_PACKAGE_IDENTITY_NAME (REQUIRED)
if(NOT DEFINED CPACK_MSIX_PACKAGE_IDENTITY_NAME)
    message(FATAL_ERROR "[CPACK MSIX] Expecting a non-empty 'CPACK_MSIX_PACKAGE_IDENTITY_NAME' value!")
endif()
# CPACK_MSIX_PACKAGE_NAME
if(NOT DEFINED CPACK_MSIX_PACKAGE_NAME)
    set(CPACK_MSIX_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
endif()
# CPACK_MSIX_PACKAGE_VENDOR
if(NOT DEFINED CPACK_MSIX_PACKAGE_VENDOR)
    set(CPACK_MSIX_PACKAGE_VENDOR ${CPACK_PACKAGE_VENDOR})
endif()
# CPACK_MSIX_PACKAGE_VENDOR_DISPLAY_NAME
if(NOT DEFINED CPACK_MSIX_PACKAGE_VENDOR_DISPLAY_NAME)
    set(CPACK_MSIX_PACKAGE_VENDOR_DISPLAY_NAME ${CPACK_MSIX_PACKAGE_VENDOR})
endif()
# CPACK_MSIX_PACKAGE_DESCRIPTION_SUMMARY
if(NOT DEFINED CPACK_MSIX_PACKAGE_DESCRIPTION_SUMMARY)
    set(CPACK_MSIX_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
endif()
# CPACK_MSIX_PACKAGE_LOGO (REQUIRED)
if((NOT CPACK_MSIX_PACKAGE_LOGO MATCHES "\\.png$") OR (NOT EXISTS ${CPACK_MSIX_PACKAGE_LOGO}))
    message(FATAL_ERROR "[CPACK MSIX] Expecting 'CPACK_MSIX_PACKAGE_LOGO' to point to a valid '.png' file!")
endif()
# CPACK_MSIX_PACKAGE_LOGO_44 (REQUIRED)
if((NOT CPACK_MSIX_PACKAGE_LOGO_44 MATCHES "\\.png$") OR (NOT EXISTS ${CPACK_MSIX_PACKAGE_LOGO_44}))
    message(FATAL_ERROR "[CPACK MSIX] Expecting 'CPACK_MSIX_PACKAGE_LOGO_44' to point to a valid 44x44 '.png' file!")
endif()
# CPACK_MSIX_PACKAGE_LOGO_150 (REQUIRED)
if((NOT CPACK_MSIX_PACKAGE_LOGO_150 MATCHES "\\.png$") OR (NOT EXISTS ${CPACK_MSIX_PACKAGE_LOGO_150}))
    message(FATAL_ERROR "[CPACK MSIX] Expecting 'CPACK_MSIX_PACKAGE_LOGO_150' to point to a valid 150x150 '.png' file!")
endif()
# CPACK_MSIX_PACKAGE_DESCRIPTION_SUMMARY
if(NOT DEFINED CPACK_MSIX_PACKAGE_DESCRIPTION_SUMMARY)
    set(CPACK_MSIX_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
endif()

# [FILE/DIR NAMES]
# CPACK_MSIX_PACKAGE_FILE_NAME
if(NOT DEFINED CPACK_MSIX_PACKAGE_FILE_NAME)
    set(CPACK_MSIX_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
endif()
# CPACK_MSIX_RUNTIME_FOLDER_NAME (Should match CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION)
if(NOT DEFINED CPACK_MSIX_RUNTIME_FOLDER_NAME)
    set(CPACK_MSIX_RUNTIME_FOLDER_NAME bin)
endif()

# [PACKAGE UPLOAD]
option(CPACK_MSIX_GENERATE_UPLOAD "Trigger MSIX '.appxupload' file generation" OFF)

# [PACKAGE COMMANDS]
# CPACK_MSIX_APPLICATIONS (REQUIRED)
if(DEFINED CPACK_MSIX_APPLICATIONS)
    set(MSIX_INTERNAL_APPLICATIONS "{ \"applications\": [${CPACK_MSIX_APPLICATIONS}] }")

    # Count apps
    string(JSON MSIX_JSON_APP_COUNT LENGTH "${MSIX_INTERNAL_APPLICATIONS}" "applications")
    math(EXPR MSIX_INTERNAL_APPLICATIONS_LAST_INDEX "${MSIX_JSON_APP_COUNT} - 1")

    # Must contain at least one app!
    if(NOT MSIX_JSON_APP_COUNT GREATER 0)
        message(FATAL_ERROR "[CPACK MSIX] Expecting at least one valid app inside 'CPACK_MSIX_APPLICATIONS'! Try using 'MSIXTools' to add applications.")
    endif()
else()
    message(FATAL_ERROR "[CPACK MSIX] Expecting 'CPACK_MSIX_APPLICATIONS' to point to a non-empty valid list of MSIX applications! Try using 'MSIXTools' to add applications.")
endif()

####################################################
## STAGING
####################################################

# Determine where CPack dumped the component folders
if(EXISTS "${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}")
    set(MSIX_INTERNAL_STAGE_ROOT "${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}")
else()
    set(MSIX_INTERNAL_STAGE_ROOT "${CPACK_TEMPORARY_DIRECTORY}")
endif()

# Create a clean staging directory
set(MSIX_STAGING_ROOT "${CPACK_TOPLEVEL_DIRECTORY}/MSIX")
file(REMOVE_RECURSE "${MSIX_STAGING_ROOT}") # Clean up from previous runs
file(MAKE_DIRECTORY "${MSIX_STAGING_ROOT}")

message(STATUS "[CPACK MSIX] Flattening component directories into ${MSIX_STAGING_ROOT}...")

# 3. Loop through the component folders and merge their contents
file(GLOB MSIX_COMPONENT_DIRS LIST_DIRECTORIES true "${MSIX_INTERNAL_STAGE_ROOT}/*")
foreach(COMP_DIR IN LISTS MSIX_COMPONENT_DIRS)
    if(IS_DIRECTORY "${COMP_DIR}")
        # Get everything inside this component folder (e.g., the 'bin' folder)
        file(GLOB COMP_CONTENTS "${COMP_DIR}/*")
        if(COMP_CONTENTS)
            # Copy it into our flattened root
            file(COPY ${COMP_CONTENTS} DESTINATION "${MSIX_STAGING_ROOT}")
        endif()
    endif()
endforeach()

####################################################
## PACKAGING READYUP
####################################################

# Get the binaries directory (usually /bin)
set(MSIX_INTERNAL_BIN "${MSIX_STAGING_ROOT}/${CPACK_MSIX_RUNTIME_FOLDER_NAME}")

# Get a list of manifest applications
set(MSIX_INTERNAL_MANIFEST_APPLICATIONS "")
foreach(INDEX RANGE ${MSIX_INTERNAL_APPLICATIONS_LAST_INDEX})
    # Extract application type
    string(JSON APP_TYPE GET "${MSIX_INTERNAL_APPLICATIONS}" "applications" ${INDEX} "type")

    if(APP_TYPE STREQUAL "ALIAS")
        # Get info
        string(JSON CURRENT_TARGET GET "${MSIX_INTERNAL_APPLICATIONS}" "applications" ${INDEX} "target")
        string(JSON CURRENT_NAME GET "${MSIX_INTERNAL_APPLICATIONS}" "applications" ${INDEX} "name")
        string(JSON CURRENT_DESCRIPTION GET "${MSIX_INTERNAL_APPLICATIONS}" "applications" ${INDEX} "description")

        message(STATUS "[CPACK MSIX] Adding 'app execution aliases' for '${CURRENT_TARGET}'...")
        set(MSIX_INTERNAL_MANIFEST_APPLICATIONS "${MSIX_INTERNAL_MANIFEST_APPLICATIONS}    <Application
                Id=\"TARGET.ALIAS.${CURRENT_TARGET}\" Executable=\"${CPACK_MSIX_RUNTIME_FOLDER_NAME}\\${CURRENT_TARGET}.exe\"
                                EntryPoint=\"Windows.FullTrustApplication\">
      
            <uap:VisualElements DisplayName=\"${CURRENT_NAME}\" Description=\"${CURRENT_DESCRIPTION}\" 
                              BackgroundColor=\"transparent\" Square150x150Logo=\"Assets\\Logo-150.png\" Square44x44Logo=\"Assets\\Logo-44.png\" />

            <Extensions>
                <uap5:Extension Category=\"windows.appExecutionAlias\">
                    <uap5:AppExecutionAlias>\n")

        # Loop through the nested aliases array
        string(JSON ALIAS_COUNT LENGTH "${MSIX_INTERNAL_APPLICATIONS}" "applications" ${INDEX} "aliases")

        if(ALIAS_COUNT GREATER 0)
            math(EXPR MSIX_INTERNAL_ALIAS_LAST_INDEX "${ALIAS_COUNT} - 1")
            foreach(ALIAS_INDEX RANGE ${MSIX_INTERNAL_ALIAS_LAST_INDEX})
                string(JSON CURRENT_ALIAS GET "${MSIX_INTERNAL_APPLICATIONS}" "applications" ${INDEX} "aliases" ${ALIAS_INDEX})
                message(STATUS "[CPACK MSIX] Adding '${CURRENT_ALIAS}' alias for '${CURRENT_TARGET}'...")

                set(MSIX_INTERNAL_MANIFEST_APPLICATIONS "${MSIX_INTERNAL_MANIFEST_APPLICATIONS}
                    <uap5:ExecutionAlias Alias=\"${CURRENT_ALIAS}.exe\" />\n")
            endforeach()
        else()
            message(STATUS "[CPACK MSIX] Target '${CURRENT_TARGET}' must have at least one alias!")
        endif()

        set(MSIX_INTERNAL_MANIFEST_APPLICATIONS "${MSIX_INTERNAL_MANIFEST_APPLICATIONS}
                    </uap5:AppExecutionAlias>
                </uap5:Extension>
            </Extensions>
        </Application>\n")
    else()
        message(FATAL_ERROR "[CPACK MSIX] Unsupported/unknown application type detected: ${APP_TYPE}")
    endif()
endforeach()

# Generate a list of files to install
#file(GLOB_RECURSE MSIX_INTERNAL_STAGED_FILES "${MSIX_STAGING_ROOT}/*")
#set(MSIX_INTERNAL_MANIFEST_FILES "")
#foreach(FILE ${MSIX_INTERNAL_STAGED_FILES})
#    # Strip the staging prefix to get the relative path
#    file(RELATIVE_PATH REL_PATH "${MSIX_STAGING_ROOT}" "${FILE}")
#    
#    # If it's a file, add it to the Manifest's <Files> section
#    if(NOT IS_DIRECTORY ${FILE})
#        set(MSIX_INTERNAL_MANIFEST_FILES "${MSIX_INTERNAL_MANIFEST_FILES}<File RelativePath=\"${REL_PATH}\" />\n    ")
#    endif()
#endforeach()

# Generate a manifest
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/AppxManifest.xml.in"
    "${MSIX_STAGING_ROOT}/AppxManifest.xml"
    @ONLY
)

# Copy manfiest assets
file(MAKE_DIRECTORY "${MSIX_STAGING_ROOT}/Assets")
file(COPY_FILE "${CPACK_MSIX_PACKAGE_LOGO}" "${MSIX_STAGING_ROOT}/Assets/Logo.png")
file(COPY_FILE "${CPACK_MSIX_PACKAGE_LOGO_44}" "${MSIX_STAGING_ROOT}/Assets/Logo-44.png")
file(COPY_FILE "${CPACK_MSIX_PACKAGE_LOGO_150}" "${MSIX_STAGING_ROOT}/Assets/Logo-150.png")

####################################################
## PACKAGING
## CREDIT: https://forum.qt.io/topic/147272/preparing-app-for-microsoft-store-qt-6-cmake?_=1737373157758
####################################################

# Find the 'makeappx' executable
set(MSIX_INTERNAL_WINDOWS_KITS_BIN_BASE "C:/Program Files (x86)/Windows Kits/10/bin")
file(GLOB MSIX_INTERNAL_VERSIONED_DIRS LIST_DIRECTORIES true "${MSIX_INTERNAL_WINDOWS_KITS_BIN_BASE}/*")
set(MSIX_INTERNAL_SEARCH_PATHS "")
foreach(DIR ${MSIX_INTERNAL_VERSIONED_DIRS})
    # Add the x64 subfolder if it exists
    if(EXISTS "${DIR}/x64")
        list(APPEND MSIX_INTERNAL_SEARCH_PATHS "${DIR}/x64")
    endif()
endforeach()
find_program(MAKEAPPX_EXECUTABLE makeappx
    PATHS ${MSIX_INTERNAL_SEARCH_PATHS}
    REQUIRED
)

# Invoke MakeAppx to bundle the directory into the final MSIX
message(STATUS "[CPACK MSIX] Packaging MSIX with MakeAppx...")
execute_process(
    COMMAND "${MAKEAPPX_EXECUTABLE}" pack 
            /d "${MSIX_STAGING_ROOT}" 
            /p "${CPACK_PACKAGE_DIRECTORY}/${CPACK_MSIX_PACKAGE_FILE_NAME}.msix"
            /o # Overwrite if exists
    RESULT_VARIABLE MAKEAPPX_RESULT
    OUTPUT_VARIABLE MAKEAPPX_OUTPUT
    ERROR_VARIABLE MAKEAPPX_ERROR
)
if(NOT MAKEAPPX_RESULT EQUAL 0)
    message(FATAL_ERROR "[CPACK MSIX] MakeAppx failed to generate the MSIX package." ${MAKEAPPX_OUTPUT} ${MAKEAPPX_ERROR})
endif()

# Check if an upload file is required
if(CPACK_MSIX_GENERATE_UPLOAD)
    message(STATUS "[CPACK MSIX] Generating a '.appxupload' file...")

    # Check for debug symbols
    file(GLOB MSIX_INTERNAL_PDB_FILES "${MSIX_INTERNAL_BIN}/*.pdb")
    list(LENGTH MSIX_INTERNAL_PDB_FILES MSIX_INTERNAL_PDB_COUNT)
    set(MSIX_INTERNAL_PDB_DETECTED OFF)
    if(MSIX_INTERNAL_PDB_COUNT GREATER 0)
        message(STATUS "[CPACK MSIX] Debug symbols found. Generating a '.appxsym' file...")
        set(MSIX_INTERNAL_PDB_DETECTED ON)

        # Zip the .pdb files into .appxsym
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar "cf" "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_MSIX_PACKAGE_FILE_NAME}.appxsym" --format=zip ${MSIX_INTERNAL_PDB_FILES}
            RESULT_VARIABLE PDB_ZIP_RESULT
        )
    endif()

    # Zip the contents into .appxupload
    if(MSIX_INTERNAL_PDB_DETECTED)
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar "cf" "${CPACK_PACKAGE_DIRECTORY}/${CPACK_MSIX_PACKAGE_FILE_NAME}.appxupload" --format=zip "${CPACK_PACKAGE_DIRECTORY}/${CPACK_MSIX_PACKAGE_FILE_NAME}.msix" "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_MSIX_PACKAGE_FILE_NAME}.appxsym"
        )
    else()
        message(WARNING "[CPACK MSIX] Couldn't include debug symbols in '.appxupload'...")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar "cf" "${CPACK_PACKAGE_DIRECTORY}/${CPACK_MSIX_PACKAGE_FILE_NAME}.appxupload" --format=zip "${CPACK_PACKAGE_DIRECTORY}/${CPACK_MSIX_PACKAGE_FILE_NAME}.msix"
        )
    endif()
endif()
