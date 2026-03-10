/**
 * @brief 
 * File-related functions
**/

#pragma once

#include "headers.hpp"
#include "dynamic.hpp" // JUG_COMMON_API

// Basic C++ headers
//#include <string>

namespace Common {
    namespace Files {
        // Check if a file is accessible
        extern JUG_COMMON_API bool isFileAccessible(const std::string &filePath) ;
        // Check if a file is of a valid format
        extern JUG_COMMON_API bool isFileValid(const std::string &filePath) ;
        // Get the contents of a file
        extern JUG_COMMON_API bool getFileContent(const std::string &filePath, std::string &store) ;
        // Get the parent folder's path
        extern JUG_COMMON_API std::string getParentPath(const std::string &path, int depth = 1) ;
        extern JUG_COMMON_API std::string getParentPath(const char *path, int depth = 1) ;
        // Get the current process executable directory
        extern JUG_COMMON_API bool getExecutableDir(std::string &store) ;
    }
}
