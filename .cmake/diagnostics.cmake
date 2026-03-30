message(STATUS "[DIAGNOSTICS] Enabling diagnostic codes generation...")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/diag_codes_generated")

set(CPP_DIAGCODES_SCAN_DIR ${CMAKE_BINARY_DIR}/..)
set(CPP_DIAGCODES_OUTPUT_DIR ${CMAKE_BINARY_DIR}/diag_codes_generated)
set(CPP_DIAGCODES_OUTPUT_FILENAME DiagCodes.hpp)
set(CPP_DIAGCODES_FILE "${CPP_DIAGCODES_OUTPUT_DIR}/${CPP_DIAGCODES_OUTPUT_FILENAME}")

# Diagnostics
if(DEFINED CPP_DIAGCODES_SCAN_DIR AND NOT TARGET JugGlobalDiagnostics)
    if(NOT DEFINED CPP_DIAGCODES_OUTPUT_DIR)
        message(FATAL_ERROR "[DEPENDENCIES] Must define a valid 'CPP_DIAGCODES_OUTPUT_DIR' value!")
    endif()
    if(NOT DEFINED CPP_DIAGCODES_OUTPUT_FILENAME)
        message(FATAL_ERROR "[DEPENDENCIES] Must define a valid 'CPP_DIAGCODES_OUTPUT_FILENAME' value!")
    endif()
    
    if(NOT DEFINED CPP_DIAGCODES_IMPORTED)
        set(CPP_DIAGCODES_IMPORTED ON)
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../utility/diag_codes ${CMAKE_BINARY_DIR}/diag_codes)
    endif()


    file(GLOB_RECURSE DIAG_FILES CONFIGURE_DEPENDS "${CPP_DIAGCODES_SCAN_DIR}/*.diag")

    add_custom_command(
        OUTPUT ${CPP_DIAGCODES_FILE}
        COMMAND JugDiagnosticsCode 
            ${CPP_DIAGCODES_OUTPUT_DIR} ${CPP_DIAGCODES_OUTPUT_FILENAME} ${CPP_DIAGCODES_SCAN_DIR}
        DEPENDS JugDiagnosticsCode ${DIAG_FILES}
        COMMENT "Running JugDiagnosticsCode to generate a diagnostic codes files..."
        VERBATIM
    )
    # Trigger the generation process
    add_custom_target(GenerateDiagCodes ALL
        DEPENDS ${CPP_DIAGCODES_FILE}
    )

    # INTERFACE library (Notice we DO NOT use target_sources here anymore!)
    if(NOT TARGET JugGlobalDiagnostics)
        add_library(JugGlobalDiagnostics INTERFACE)
        target_include_directories(JugGlobalDiagnostics INTERFACE ${CPP_DIAGCODES_OUTPUT_DIR})

        add_dependencies(JugGlobalDiagnostics GenerateDiagCodes)
    endif()
endif()
