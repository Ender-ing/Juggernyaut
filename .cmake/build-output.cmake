# Check the build mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Store output files in the 'Debug' subfolder
    set(JUG_BINARY_MODE "Debug")
    message(STATUS "[BUILD] Build set to 'Debug' mode.")
else()
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        message(WARNING "[BUILD] Invalid build mode detected! Defaulting to the 'Release' build mode.")
    else()
        message(STATUS "[BUILD] Detected the 'Release' build mode.")
    endif()
    # Store output files in the 'Release' subfolder
    set(JUG_BINARY_MODE "Release")
    message(STATUS "[BUILD] Build set to 'Release' mode.")
endif()

# Detect the platform
get_build_arch(JUG_GENERATOR_PLATFORM)

# Get the current build architecture
project(ArchitectureSpecificBuild)
if(WIN32)
    if(${JUG_GENERATOR_PLATFORM} STREQUAL "x86")
        set(JUG_BINARY_PLATFORM "x86_32")
    elseif(${JUG_GENERATOR_PLATFORM} STREQUAL "x64")
        set(JUG_BINARY_PLATFORM "x86_64")
    elseif(${JUG_GENERATOR_PLATFORM} STREQUAL "ARM")
        set(JUG_BINARY_PLATFORM "arm32")
    elseif(${JUG_GENERATOR_PLATFORM} STREQUAL "ARM64")
        set(JUG_BINARY_PLATFORM "arm64")
    else()
        message(FATAL_ERROR "[BUILD] Couldn't determine the target architecture for the build on Windows.")
    endif()
elseif(UNIX)
    if(${JUG_GENERATOR_PLATFORM} STREQUAL "x86")
        if(CMAKE_SYSTEM_NAME MATCHES "Linux")
            #add_c_cpp_global_flag("-m32")
            #add_c_cpp_global_flag("-march=i686")

            # Include/find directories
            #set(CMAKE_FIND_ROOT_PATH /usr/lib/i686-linux-gnu /usr/include)
            #set(CMAKE_FIND_LIBRARY_PATHS /usr/lib/i686-linux-gnu)
            #set(CMAKE_FIND_INCLUDE_PATH /usr/include)

            # Root path settings
            #set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
            #set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
            #set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
        elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
            message(FATAL_ERROR "[BUILD] 32-bit builds are not supported on macOS!")
        endif()
        set(JUG_BINARY_PLATFORM "x86_32")
    elseif(${JUG_GENERATOR_PLATFORM} STREQUAL "x64")
        if(CMAKE_SYSTEM_NAME MATCHES "Linux")
            add_c_cpp_global_flag("-m64")
            #add_c_cpp_global_flag("-march=x86-64")
        elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
            add_c_cpp_global_flag("-arch x86_64")
            set(CMAKE_OSX_ARCHITECTURES "x86_64")
        endif()
        set(JUG_BINARY_PLATFORM "x86_64")
    elseif(${JUG_GENERATOR_PLATFORM} STREQUAL "ARM")
        if(CMAKE_SYSTEM_NAME MATCHES "Linux")
            #add_c_cpp_global_flag("-marm")
            add_c_cpp_global_flag("-march=armv7-a")
            #set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
            #set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
        elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
            message(FATAL_ERROR "[BUILD] 32-bit builds are not supported on macOS!")
        endif()
        set(JUG_BINARY_PLATFORM "arm32")
    elseif(${JUG_GENERATOR_PLATFORM} STREQUAL "ARM64")
        if(CMAKE_SYSTEM_NAME MATCHES "Linux")
            #add_c_cpp_global_flag("-mabi=lp64")
            #add_c_cpp_global_flag("-mfpu=neon-fp-armv8")
            add_c_cpp_global_flag("-march=armv8-a")
        elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
            add_c_cpp_global_flag("-arch arm64")
            set(CMAKE_OSX_ARCHITECTURES "arm64")
        endif()
        set(JUG_BINARY_PLATFORM "arm64")
    else()
        message(FATAL_ERROR "[BUILD] Couldn't determine the target architecture for the build on your Unix-like system.")
    endif()
else()
    message(FATAL_ERROR "[BUILD] Unsupported target architecture/system.")
endif()

# Set the desired CMake binary output directory
set(JUG_DIST_FINAL_DIR ${JUG_BINARY_DIR}/${JUG_BINARY_MODE}/${JUG_BINARY_PLATFORM})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${JUG_DIST_FINAL_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${JUG_DIST_FINAL_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${JUG_DIST_FINAL_DIR}/lib)

# Clear configuration-specific variables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${JUG_BINARY_DIR}/Debug/${JUG_BINARY_PLATFORM}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${JUG_BINARY_DIR}/Release/${JUG_BINARY_PLATFORM}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${JUG_BINARY_DIR}/Debug/${JUG_BINARY_PLATFORM}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${JUG_BINARY_DIR}/Release/${JUG_BINARY_PLATFORM}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${JUG_BINARY_DIR}/Debug/${JUG_BINARY_PLATFORM}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${JUG_BINARY_DIR}/Release/${JUG_BINARY_PLATFORM}/lib)

# Generate compile command database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Attach manifest.ini info to targets!
# (Only supports executables and dynamic libraries!)
set(NEEDS_WIN_RC ON)
function(attach_manifest_data TARGET MANIFEST LINK_INFO)

    message(STATUS "[BUILD] Attaching manifest data to target: ${TARGET}")
    # Set versioning info
    get_ini_value(${MANIFEST} "TARGET:${TARGET}" "VERSION" INI_VERSION)
    #get_ini_value(${MANIFEST} "TARGET:${TARGET}" "SOVERSION" INI_SOVERSION)
    if(INI_VERSION STREQUAL "NO_ARG_VAL")
        set(INI_VERSION "0.0.0-cmake.000")
    endif()

    # Update target info
    set_target_properties(${TARGET} PROPERTIES # THIS DOESN'T WORK!
        VERSION "${INI_VERSION}"
        #SOVERSION "${INI_SOVERSION}" # CMAKE LINKS TO THIS, BEFORE "VERSION"
        # Naming scheme
        OUTPUT_NAME "${TARGET}"
    )

    # Keep track of the main target value
    if(NOT DEFINED IN_MAIN_BIN_VERSION_NAME)
        # Save version info
        set(IN_MAIN_BIN_VERSION_NAME "${INI_VERSION}")
        set(IN_MAIN_BIN_VERSION_NAME "${IN_MAIN_BIN_VERSION_NAME}" PARENT_SCOPE)
        string(REGEX MATCHALL "[0-9]+" version_list "${INI_VERSION}")
        list(GET version_list 0 IN_MAIN_BIN_MAJOR)
        set(IN_MAIN_BIN_MAJOR "${IN_MAIN_BIN_MAJOR}" PARENT_SCOPE)
        list(GET version_list 1 IN_MAIN_BIN_MINOR)
        set(IN_MAIN_BIN_MINOR "${IN_MAIN_BIN_MINOR}" PARENT_SCOPE)
        list(GET version_list 2 IN_MAIN_BIN_PATCH)
        set(IN_MAIN_BIN_PATCH "${IN_MAIN_BIN_PATCH}" PARENT_SCOPE)
        list(GET version_list 3 IN_MAIN_BIN_EXTRA)
        set(IN_MAIN_BIN_EXTRA "${IN_MAIN_BIN_EXTRA}" PARENT_SCOPE)
    endif()

    # Pass manifest data to .in file
    string(REGEX MATCHALL "[0-9]+" version_list "${INI_VERSION}")
    list(GET version_list 0 IN_BIN_MAJOR)
    list(GET version_list 1 IN_BIN_MINOR)
    list(GET version_list 2 IN_BIN_PATCH)
    list(GET version_list 3 IN_BIN_EXTRA)
    get_ini_value(${MANIFEST} "TARGET:${TARGET}" "DESCRIPTION" IN_BIN_DESCRIPTION)
    set(IN_BIN_VERSION_NAME ${INI_VERSION})

    # Get target type
    get_target_property(target_type ${TARGET} TYPE)

    # Link native platform
    if(LINK_INFO)
        if(WIN32)
            # Modify binary name
            if(target_type STREQUAL "SHARED_LIBRARY")
                set(IN_BIN_NAME "${TARGET}.dll")
            else()
                set(IN_BIN_NAME "${TARGET}.exe")
            endif()
            # Compile Windows resource script file
            set(WIN32_VERSIONING_RC_FILE ${JUG_BUILD_DIR}/versioning/${TARGET}.Rc)
            set(WIN32_VERSIONING_RES_FILE ${JUG_BUILD_DIR}/versioning/${TARGET}.res)
            configure_file(${JUG_CMAKE_DIR}/version/VersionInfo.Rc.in ${WIN32_VERSIONING_RC_FILE})
            execute_process(
                COMMAND ${RC_EXECUTABLE} /fo ${WIN32_VERSIONING_RES_FILE} ${WIN32_VERSIONING_RC_FILE}
                RESULT_VARIABLE command_result
            )
            if(NOT command_result EQUAL 0)
                message(FATAL_ERROR "[BUILD] Couldn't compile Windows resource file: ${WIN32_VERSIONING_RES_FILE}")
            endif()
            # Link Windows resource file to target
            target_link_libraries(${TARGET} PRIVATE ${WIN32_VERSIONING_RES_FILE})
        elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
            # Modify binary name
            if(target_type STREQUAL "SHARED_LIBRARY")
                set(IN_BIN_NAME "lib${TARGET}.${IN_BIN_VERSION_NAME}.dylib")
            else()
                set(IN_BIN_NAME "${TARGET}.${IN_BIN_VERSION_NAME}")
            endif()
        elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
            # Modify binary name
            if(target_type STREQUAL "SHARED_LIBRARY")
                set(IN_BIN_NAME "lib${TARGET}.so.${IN_BIN_VERSION_NAME}")
            else()
                set(IN_BIN_NAME "${TARGET}.${IN_BIN_VERSION_NAME}")
            endif()
        endif()
    endif()

    # Pass data to the target's C++ source files
    target_compile_definitions(${TARGET} PRIVATE
        "MAIN_TARGET_BINARY_VERSION=\"${IN_MAIN_BIN_VERSION_NAME}\""
        "TARGET_BINARY_VERSION=\"${IN_BIN_VERSION_NAME}\""
    )
endfunction()

# Manage symbolic links post-build
function(manage_symbolic_links POST_TARGET JUG_COMMAND_NAME)
    if(WIN32)
        set(SYMBOLIC_LINKS_COMMAND_DELETE del /f /q /s ${JUG_COMMAND_NAME}.exe)
        set(SYMBOLIC_LINKS_COMMAND_REMAKE echo "Windows already has a valid .exe main file!")
        set(SYMBOLIC_LINKS_COMMAND_EXTRA mklink ${JUG_COMMAND_NAME}.exe ${POST_TARGET}.exe)
    else()
        set(SYMBOLIC_LINKS_COMMAND_DELETE rm -f ${JUG_COMMAND_NAME} ${POST_TARGET})
        set(SYMBOLIC_LINKS_COMMAND_REMAKE ln -s ${POST_TARGET}* ${POST_TARGET})
        set(SYMBOLIC_LINKS_COMMAND_EXTRA ln -s ${POST_TARGET} ${JUG_COMMAND_NAME})
    endif()
    add_custom_command(
        TARGET ${POST_TARGET}
        POST_BUILD
        COMMAND ${SYMBOLIC_LINKS_COMMAND_DELETE}
        COMMAND ${SYMBOLIC_LINKS_COMMAND_REMAKE}
        COMMAND ${SYMBOLIC_LINKS_COMMAND_EXTRA}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endfunction()
function(create_symbolic_link TARGET SOURCE LINK IS_LIB)
    if(WIN32)
        if(IS_LIB)
            set(SYMBOLIC_LINKS_COMMAND_LINK mklink ${LINK}.dll ${SOURCE}.dll)
        else()
            set(SYMBOLIC_LINKS_COMMAND_LINK mklink ${LINK}.exe ${SOURCE}.exe)
        endif()
    else()
        set(SYMBOLIC_LINKS_COMMAND_LINK ln -s ${SOURCE} ${LINK})
    endif()
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        #COMMAND ${SYMBOLIC_LINKS_COMMAND_DELETE}
        COMMAND ${SYMBOLIC_LINKS_COMMAND_LINK}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endfunction()

# Post-build files
function(delete_file TARGET FILE)
    if(WIN32)
        set(DELETE_COMMAND del /f /q /s ${FILE})
    else()
        set(DELETE_COMMAND rm -f ${FILE})
    endif()
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${DELETE_COMMAND}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endfunction()
function(rename_file TARGET OLD_PATH NEW_PATH)
    if(WIN32)
        set(RENAME_COMMAND ren ${OLD_PATH} ${NEW_PATH})
    else()
        set(RENAME_COMMAND mv ${OLD_PATH} ${NEW_PATH})
    endif()
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${RENAME_COMMAND}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endfunction()

# Fix dynamic libraries
function(copy_shared_library FUNC_TARGET LIB_PATH LIB_VERSION)
    # Libraries
    add_custom_command(TARGET ${FUNC_TARGET}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND}
                           -E copy ${LIB_PATH} .
                    WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    # Fix antlr4-runtime library naming!
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        get_filename_component(LIB_FILENAME "${LIB_PATH}" NAME)
        delete_file(${FUNC_TARGET} "${LIB_FILENAME}.${LIB_VERSION}")
        rename_file(${FUNC_TARGET} ${LIB_FILENAME} "${LIB_FILENAME}.${LIB_VERSION}")
        create_symbolic_link(${FUNC_TARGET} "${LIB_FILENAME}.${LIB_VERSION}" ${LIB_FILENAME} FALSE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        get_filename_component(LIB_FILENAME_NOEXT "${LIB_PATH}" NAME_WE)
        delete_file(${FUNC_TARGET} "${LIB_FILENAME_NOEXT}.${LIB_VERSION}.dylib")
        rename_file(${FUNC_TARGET} "${LIB_FILENAME_NOEXT}.dylib" "${LIB_FILENAME_NOEXT}.${LIB_VERSION}.dylib")
        create_symbolic_link(${FUNC_TARGET} "${LIB_FILENAME_NOEXT}.${LIB_VERSION}.dylib" "${LIB_FILENAME_NOEXT}.dylib" FALSE)
    endif()
endfunction()

function(copy_proper_shared_library FUNC_TARGET SRC DEST)
    add_custom_command(TARGET ${FUNC_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
            -DSRC_DIR="${SRC}" 
            -DDST_DIR="${DEST}" 
            -P "${JUG_CMAKE_DIR}/components/copy_shared_libs.cmake"
        COMMENT "Gathering dynamic libraries..."
    )
endfunction()
