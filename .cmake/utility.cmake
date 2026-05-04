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
                set(VALUE "${${ARG_INHERIT}}")
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

macro(generate_command SOURCE_NAME)
    if(WIN32)
        configure_file(
            "${JUG_CMAKE_DIR}/components/${SOURCE_NAME}.bat.in"
            "${JUG_DIST_FINAL_DIR}.${SOURCE_NAME}.bat"
            @ONLY # Only replace @VAR@ style variables
        )
    else()
        # Generates a .sh file for Linux/macOS users
        configure_file(
            "${JUG_CMAKE_DIR}/components/${SOURCE_NAME}.sh.in"
            "${JUG_DIST_FINAL_DIR}.${SOURCE_NAME}.sh"
            @ONLY
            FILE_PERMISSIONS 
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
        )
    endif()
endmacro()
