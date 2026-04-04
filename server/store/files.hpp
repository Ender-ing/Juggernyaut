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
    extern bool isFileAccessible(const std::string &filePath) ;
    // Check if the file is of a valid format
    extern bool isFileValid(const std::string &filePath) ;
    extern bool getFileContent(const std::string &filePath, std::string &store) ;
}
