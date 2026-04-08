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
    extern std::string normalizePath(const std::string &path) ;
    extern bool isFileAccessible(const std::string &filePath) ;
    extern bool isValidDir(const std::string &path) ;
    extern std::string getFileExtension(const std::string &filePath) ;
    extern std::string getParentPath(const std::string &filePath) ;
    extern std::string joinPaths(const std::string &base, const std::string path) ;
    extern bool getFileContent(const std::string &filePath, std::string &store) ;
}
