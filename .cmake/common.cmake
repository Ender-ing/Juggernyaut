message(STATUS "[COMMON] Including global common library...")

include(FetchContent)

if(NOT TARGET JuggernyautCommonLibrary)
    if(NOT DEFINED CPP_COMMON_IMPORTED)
        set(CPP_COMMON_IMPORTED ON)
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../utility/common ${CMAKE_BINARY_DIR}/common)
    endif()
    link_libraries(JuggernyautCommonLibrary)
endif()
