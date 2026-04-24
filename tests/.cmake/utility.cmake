# Limit builds
macro(dep_check)
    if(NOT DEFINED JUG_TESTS_BUILD)
        message(FATAL_ERROR "[TESTS] Test units must be configured through the main CMakeLists.txt file!")
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
        JuggernyautCommonLibrary # Just in case (also, it should already be linked anyways...)
        GTest::gtest # Or gtest_main
    )
    
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
endmacro()
