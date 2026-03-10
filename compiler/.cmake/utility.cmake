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
        if(INIT_INHERIT)
            string(REGEX REPLACE "INHERIT:(.*)" "\\1" INIT_INHERIT "${INIT_INHERIT}")
            # Get value from INIT_INHERIT's section
            get_ini_value(${INI_FILE} ${INIT_INHERIT} ${KEY} VALUE)
        endif()
    else()
        message(FATAL_ERROR "[UTILITY] Unexpected value assigned to key '${KEY}' from section '${INI_SECTION}' in file '${INI_FILE}'")
    endif()

    # Return value
    set(${OUTPUT_VARIABLE} ${VALUE} PARENT_SCOPE)
endfunction()
