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
    extern std::string normalizePath(const std::string &path) ;
    extern bool isFileAccessible(const std::string &filePath) ;
    extern std::string getFileExtension(const std::string &filePath) ;
    extern bool getFileContent(const std::string &filePath, std::string &store) ;
    extern std::string getParentPath(const std::string &path) ;
    extern std::string joinPaths(const std::string &base, const std::string path) ;
    // Get the current process executable directory
    extern bool getExecutableDir(std::string &store) ;
}
