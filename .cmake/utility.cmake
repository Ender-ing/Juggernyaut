# Files search
function(target_sources_search TARGET FILE_PATHS IS_RECURSIVE)
    # Choose search type
    if(IS_RECURSIVE)
        file(GLOB_RECURSE SEARCH_FILES ${FILE_PATHS})
    else()
        file(GLOB SEARCH_FILES ${FILE_PATHS})
    endif()
    # Add files to the target's sources
    foreach(file_path ${SEARCH_FILES})
        message(STATUS "[BUILD] Adding '${TARGET}' target source: '${file_path}'")
        target_sources(${TARGET}
            PRIVATE
            ${file_path}
        )
    endforeach()
endfunction()

# .ini value reader
function(get_ini_value INI_FILE INI_SECTION KEY OUTPUT_VARIABLE)
    # Get file contents
    file(READ ${INI_FILE} INI_CONTENTS)
    string(REGEX REPLACE "\r" "" INI_CONTENTS "${INI_CONTENTS}")

    # Get the required section
    string(REGEX MATCH "(\\[${INI_SECTION}\\]\n)(([^\\[]*)\n)*\\[?" MATCHED_SECTION "${INI_CONTENTS}")
    string(LENGTH "${MATCHED_SECTION}" section_length)
    if (NOT section_length GREATER 0)
        message(FATAL_ERROR "[UTILITY] Couldn't find section '${INI_SECTION}' in file '${INI_FILE}'")
    endif()

    # Look for the key
    string(REGEX MATCH "(^|\n)(${KEY}\\=[^\n]*)" MATCHED_VALUE "${MATCHED_SECTION}")
    string(LENGTH "${MATCHED_VALUE}" matched_value_length)
    if (NOT matched_value_length GREATER 0)
        message(FATAL_ERROR "[UTILITY] Couldn't find the key '${KEY}' in section '${INI_SECTION}' in file '${INI_FILE}'")
    endif()

    # Check the key type
    if(MATCHED_VALUE MATCHES "${KEY}\\=[\\s]*[0-9]")
        # Integer value

        # Get a basic numerical value
        string(REGEX REPLACE "(^|\n)${KEY}\\=[\\s]*(([0-9]+))[^\n]*\n?" "\\3" VALUE "${MATCHED_VALUE}")
        string(STRIP "${VALUE}" VALUE) # Remove extra whitespace (for bad string values)

        # Convert to an integer
        math(EXPR VALUE "${VALUE}")
    elseif(MATCHED_VALUE MATCHES "${KEY}\\=[\\s]*\"")
        # String value

        # Only get the value
        string(REGEX REPLACE "(^|\n)${KEY}\\=[\\s]*(\"(.*)\")[^\n]*\n?" "\\3" VALUE "${MATCHED_VALUE}")
        string(STRIP "${VALUE}" VALUE) # Remove extra whitespace (for bad string values)

        # Check for value inheritance
        string(REGEX MATCH "^INHERIT:.+" INIT_INHERIT "${VALUE}")
        string(REGEX MATCH "^ARG:.+" ARG_INHERIT "${VALUE}")
        if(INIT_INHERIT)
            string(REGEX REPLACE "INHERIT:(.*)" "\\1" INIT_INHERIT "${INIT_INHERIT}")
            # Get value from INIT_INHERIT's section
            get_ini_value(${INI_FILE} ${INIT_INHERIT} ${KEY} VALUE)
        elseif(ARG_INHERIT)
            string(REGEX REPLACE "ARG:(.*)" "\\1" ARG_INHERIT "${ARG_INHERIT}")
            if(DEFINED ${ARG_INHERIT})
                string(TOLOWER "${${ARG_INHERIT}}" VALUE)
            else()
                set(VALUE "NO_ARG_VAL")
            endif()
            set(VALUE "${VALUE}" PARENT_SCOPE)
        endif()
    else()
        message(FATAL_ERROR "[UTILITY] Unexpected value assigned to key '${KEY}' from section '${INI_SECTION}' in file '${INI_FILE}'")
    endif()

    # Return value
    set(${OUTPUT_VARIABLE} ${VALUE} PARENT_SCOPE)
endfunction()

function(get_build_arch OUTPUT_VARIABLE)

    # First, try to follow the generator argument
    if((NOT "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "") AND WIN32)
        if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")
            set(${OUTPUT_VARIABLE} "x86" PARENT_SCOPE)
        elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
            set(${OUTPUT_VARIABLE} "x64" PARENT_SCOPE)
        elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "ARM")
            set(${OUTPUT_VARIABLE} "ARM" PARENT_SCOPE)
        elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "ARM64")
            set(${OUTPUT_VARIABLE} "ARM64" PARENT_SCOPE)
        else()
            message(FATAL_ERROR "[UTILITY] Unknown 'CMAKE_GENERATOR_PLATFORM' value: ${CMAKE_GENERATOR_PLATFORM}")
        endif()
    elseif((NOT "${CMAKE_UNIX_GENERATOR_PLATFORM}" STREQUAL "") AND UNIX)
        if("${CMAKE_UNIX_GENERATOR_PLATFORM}" STREQUAL "x86")
            set(${OUTPUT_VARIABLE} "x86" PARENT_SCOPE)
        elseif("${CMAKE_UNIX_GENERATOR_PLATFORM}" STREQUAL "x64")
            set(${OUTPUT_VARIABLE} "x64" PARENT_SCOPE)
        elseif("${CMAKE_UNIX_GENERATOR_PLATFORM}" STREQUAL "ARM")
            set(${OUTPUT_VARIABLE} "ARM" PARENT_SCOPE)
        elseif("${CMAKE_UNIX_GENERATOR_PLATFORM}" STREQUAL "ARM64")
            set(${OUTPUT_VARIABLE} "ARM64" PARENT_SCOPE)
        else()
            message(FATAL_ERROR "[UTILITY] Unknown 'CMAKE_UNIX_GENERATOR_PLATFORM' value: ${CMAKE_UNIX_GENERATOR_PLATFORM}")
        endif()
    else()
        string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" TARGET_ARCH)

        # Detect ARM vs x86 Architectures
        if(TARGET_ARCH MATCHES "aarch64|arm64")
            set(${OUTPUT_VARIABLE} "ARM64" PARENT_SCOPE)
        elseif(TARGET_ARCH MATCHES "arm|armv7|armv7s|armv7l")
            set(${OUTPUT_VARIABLE} "ARM" PARENT_SCOPE)
        elseif(TARGET_ARCH MATCHES "x86_64|amd64")
            set(${OUTPUT_VARIABLE} "x64" PARENT_SCOPE)
        elseif(TARGET_ARCH MATCHES "i386|i686|x86")
            set(${OUTPUT_VARIABLE} "x86" PARENT_SCOPE)
        else()
            message(FATAL_ERROR "[UTILITY] Unknown Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    endif()
endfunction()

macro(custom_malloc TARGET)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (${TARGET} MATCHES "^Juggernyaut")
            add_dependencies(${TARGET} mimalloc)
        endif()
        target_link_libraries(${TARGET} INTERFACE mimalloc)
    endif()
endmacro()

macro(install_command_symlink TARGET_NAME COMPONENT_NAME LINK_NAME)
    if(WIN32)
        install(CODE "
            execute_process(
                COMMAND cmd /c del /f /q \"${LINK_NAME}.exe\"
                WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/bin\"
            )
            execute_process(
                COMMAND cmd /c mklink \"${LINK_NAME}.exe\" \"${TARGET_NAME}.exe\"
                WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/bin\"
                RESULT_VARIABLE _link_result
            )
            if(NOT _link_result EQUAL 0)
                message(FATAL_ERROR \"Couldn't generate the symbolic link '${LINK_NAME}.exe' for ${TARGET_NAME}.exe!\")
            endif()
        " COMPONENT ${COMPONENT_NAME})
    else()
        install(CODE "
            set(DESTDIR_PREFIX \"\$ENV{DESTDIR}\")
            set(FULL_INSTALL_DIR \"\${DESTDIR_PREFIX}\${CMAKE_INSTALL_PREFIX}/bin\")
            execute_process(
                COMMAND \${CMAKE_COMMAND} -E rm -f \"${LINK_NAME}\"
                WORKING_DIRECTORY \"\${FULL_INSTALL_DIR}\"
            )
            execute_process(
                COMMAND \${CMAKE_COMMAND} -E create_symlink \"${TARGET_NAME}\" \"${LINK_NAME}\"
                WORKING_DIRECTORY \"\${FULL_INSTALL_DIR}\"
                RESULT_VARIABLE _link_result
            )
            if(NOT _link_result EQUAL 0)
                message(FATAL_ERROR \"Couldn't generate the symbolic link '${LINK_NAME}' for ${TARGET_NAME}!\")
            endif()
        " COMPONENT ${COMPONENT_NAME})
    endif()
endmacro()

function(sanitize_version RAW_VERSION OUTPUT_VAR)
    # Look for version patterns
    if(RAW_VERSION MATCHES "^([0-9]+\\.[0-9]+\\.[0-9]+)-([A-Za-z_]*)\\.([0-9][0-9][0-9])$")
        set(SEMVER ${CMAKE_MATCH_1})
        set(REL_NAME ${CMAKE_MATCH_2})
        set(REL_PATCH ${CMAKE_MATCH_3})

        string(TOLOWER "${REL_NAME}" REL_NAME)
        if(REL_NAME STREQUAL "alpha")
            set(REL_CODE "1")
        elseif(REL_NAME STREQUAL "beta")
            set(REL_CODE "2")
        elseif(REL_NAME STREQUAL "nightly")
            set(REL_CODE "3")
        elseif(REL_NAME STREQUAL "candidate")
            set(REL_CODE "4")
        elseif(REL_NAME STREQUAL "release")
            set(REL_CODE "5")
        elseif(REL_NAME STREQUAL "hotfix")
            set(REL_CODE "6")
        else()
            set(REL_CODE "0") # Fallback for unknown release tags
        endif()

        # Construct version string
        set(${OUTPUT_VAR} "${SEMVER}-${REL_CODE}.${REL_PATCH}" PARENT_SCOPE)

    else()
        message(FATAL_ERROR "[VERSION SANITIZER] Release version string pattern isn't allowed!")
    endif()
endfunction()
