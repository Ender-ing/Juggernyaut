# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

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

# Create a library from / (the root directory of /compiler)
add_library(JuggernyautBaseLibrary SHARED)
target_sources_search(JuggernyautBaseLibrary ${JUG_SOURCE_DIR}/base.*.cpp FALSE)
# Expose library exports
target_compile_definitions(JuggernyautBaseLibrary PRIVATE JUG_BASE_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautBaseLibrary)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautBaseLibrary FALSE)
# Link other libraries to the library
add_dependencies(JuggernyautBaseLibrary JuggernyautCommonLibrary)
target_link_libraries(JuggernyautBaseLibrary PUBLIC JuggernyautCommonLibrary)
add_dependencies(JuggernyautBaseLibrary JuggernyautCommsLibrary)
target_link_libraries(JuggernyautBaseLibrary PUBLIC JuggernyautCommsLibrary)

# Create a library from /comms
add_library(JuggernyautCommsLibrary SHARED)
target_sources_search(JuggernyautCommsLibrary ${JUG_SOURCE_DIR}/comms/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautCommsLibrary PRIVATE JUG_COMMS_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautCommsLibrary)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCommsLibrary FALSE)
# Link other libraries to the library
add_dependencies(JuggernyautCommsLibrary fmt::fmt)
target_link_libraries(JuggernyautCommsLibrary PUBLIC fmt::fmt)

# Create a library from /common
add_library(JuggernyautCommonLibrary SHARED)
target_sources_search(JuggernyautCommonLibrary ${JUG_SOURCE_DIR}/common/*.cpp TRUE)
# Expose library exports
target_compile_definitions(JuggernyautCommonLibrary PRIVATE JUG_COMMON_LIBRARY_EXPORTS)
# Attach manifest data
attach_manifest_data(JuggernyautCommonLibrary)
# Add compiler flags
add_internal_target_cxx_flags(JuggernyautCommonLibrary FALSE)

# Set the project libraries
set(PROJECT_LIBRARIES
    JuggernyautBaseLibrary
    JuggernyautCommonLibrary
    JuggernyautCommsLibrary
)
