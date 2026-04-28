# Limit builds
macro(dep_check)
    if(NOT DEFINED JUG_TESTS_BUILD)
        message(FATAL_ERROR "[TESTS] Test units must be configured through the main CMakeLists.txt file!")
    endif()
endmacro()

# Linking libraries
macro(link_external_test_target CURR_TARGET EXTERNAL)
    target_link_libraries(${CURR_TARGET}
        PRIVATE
        ${EXTERNAL}
    )
    if(WIN32)
        add_custom_command(TARGET ${CURR_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${EXTERNAL}>"
            "$<TARGET_FILE_DIR:${CURR_TARGET}>"
        )
    endif()
endmacro()

# Defining targets
macro(add_test_target TARGET DIR)
    message(STATUS "[BUILD] Adding '${TARGET}' tests...")
    add_executable(${TARGET}
        ${DIR}/tests_main.cpp
    )

    # Link against gtest and required libraries
    target_link_libraries(${TARGET}
        PRIVATE
        GTest::gtest # Or gtest_main
    )
    link_external_test_target(${TARGET} JuggernyautCommonLibrary)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        link_external_test_target(${TARGET} mimalloc)
    endif()
    
    # Attach manifest data
    attach_manifest_data(${TARGET} ${JUG_TESTS_MANIFEST_FILE} TRUE)

    # Add compiler flags
    add_internal_target_cxx_flags(${TARGET} FALSE)

    # Add debug flags
    include(${JUG_CMAKE_DIR}/debug-mode.cmake)

    # Expose to the test command
    gtest_discover_tests(${TARGET})

    # FIX: Ignore known warnings caused by googletest
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /wd6326)
    endif()

    # Handle local library linking
    if(NOT WIN32)
        set_target_properties(${TARGET} PROPERTIES 
            BUILD_RPATH "$ORIGIN/../../bin"
            INSTALL_RPATH "$ORIGIN/../../bin"
        )
    endif()
endmacro()
