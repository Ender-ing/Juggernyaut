/**
 * @brief 
 * File-related functions
**/

#pragma once

#include "../../core/common/headers.hpp"

// Basic C++ headers
#include <filesystem>

namespace fs = std::filesystem;

namespace Store {
    extern fs::path normalizePath(fs::path &path) ;
    // Check if a file is accessible
    extern bool isFileAccessible(const std::string &filePath) ;
    // Check if a file is of a valid format
    extern bool isFileValid(const std::string &filePath) ;
    // Get the contents of a file
    extern bool getFileContent(const std::string &filePath, std::string &store) ;
    // Get the parent folder's path
    extern std::string getParentPath(const std::string &path, int depth = 1) ;
    extern std::string getParentPath(const char *path, int depth = 1) ;
    // Get the current process executable directory
    extern bool getExecutableDir(std::string &store) ;
}
