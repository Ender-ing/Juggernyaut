# Manage C++ compiler settings
if (NOT DEFINED add_c_cpp_global_flag)
    include(${JUG_CMAKE_DIR}/compiler/cpp.cmake)
endif()

# CMake functions & macros
if (NOT DEFINED target_sources_search)
    include(${JUG_CMAKE_DIR}/utility.cmake)
endif()
