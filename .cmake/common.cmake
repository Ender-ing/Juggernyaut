message(STATUS "[COMMON] Including global common library...")

include(FetchContent)

if(NOT TARGET JuggernyautCommonLibrary)
    if(NOT DEFINED JUG_COMMON_INCLUDE_DIR)
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../utility/common ${CMAKE_BINARY_DIR}/common)
    endif()
    link_libraries(JuggernyautCommonLibrary)
    #include_directories(${JUG_COMMON_INCLUDE_DIR})
endif()
