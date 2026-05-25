message(STATUS "[LIBRARIES] Including global libraries...")

include(FetchContent)

if(NOT TARGET JuggernyautCommonLibrary)
    # (using the common library for detection)
    if(NOT DEFINED JUG_COMMON_INCLUDE_DIR)
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../utility/libraries ${CMAKE_BINARY_DIR}/libraries)
    endif()
endif()

macro(jug_common TARGET)
    add_dependencies(${TARGET} JuggernyautCommonLibrary)
    target_link_libraries(${TARGET} PRIVATE JuggernyautCommonLibrary)
    custom_malloc(${TARGET})
endmacro()
