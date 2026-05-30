cmake_minimum_required(VERSION 4.2 FATAL_ERROR)

####################################################
## USE
####################################################

# include(".../MSIXTools.cmake")

####################################################
## Application alias definition
## INPUT:
##      - TARGET (The name of the executable in your runtime directory)
##      - DISPLAY NAME (The name shown in system UIs)
##      - DESCRIPTION
##      - ALIASES (accessed through the command line)
####################################################
function(cpack_msix_add_application_alias TARGET DISPLAY_NAME DESCRIPTION)
    set(CPACK_MSIX_APPLICATIONS_MOD)
    if((NOT DEFINED CPACK_MSIX_APPLICATIONS) OR (CPACK_MSIX_APPLICATIONS STREQUAL ""))
        set(CPACK_MSIX_APPLICATIONS_MOD "{\n")
    else()
        set(CPACK_MSIX_APPLICATIONS_MOD "${CPACK_MSIX_APPLICATIONS},{\n")
    endif()
    set(CPACK_MSIX_APPLICATIONS_MOD "${CPACK_MSIX_APPLICATIONS_MOD}
    \\\"type\\\": \\\"ALIAS\\\",
    \\\"target\\\": \\\"${TARGET}\\\",
    \\\"name\\\": \\\"${DISPLAY_NAME}\\\",
    \\\"description\\\": \\\"${DESCRIPTION}\\\",
    \\\"aliases\\\": [")
    set(ALIAS_LIST "")
    foreach(ALIAS IN LISTS ARGN)
        if(ALIAS_LIST STREQUAL "")
            set(ALIAS_LIST "\\\"${ALIAS}\\\"")
        else()
            set(ALIAS_LIST "${ALIAS_LIST}, \\\"${ALIAS}\\\"")
        endif()
    endforeach()
    set(CPACK_MSIX_APPLICATIONS_MOD "${CPACK_MSIX_APPLICATIONS_MOD}${ALIAS_LIST}]}")

    # Update CPACK_MSIX_APPLICATIONS
    set(CPACK_MSIX_APPLICATIONS "${CPACK_MSIX_APPLICATIONS_MOD}" PARENT_SCOPE)
endfunction()
