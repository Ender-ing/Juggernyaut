/**
 * @brief
 * File-related functions
**/

#include "files.hpp"

// Basic C++ headers
#include <fstream>

namespace fs = std::filesystem;

namespace Store {
    bool isFileAccessible(const std::string &filePath) {
        // Check if the file is open!
        std::ifstream file(filePath);
        return file.is_open(); // FIle will be closed when out of scope
    }
    std::string getFileExtension(const std::string &filePath) {
        // Check file extension
        fs::path filePathObj = filePath;
        return filePathObj.extension().string();
    }
    std::string getParentPath(const std::string &filePath) {
        fs::path localPath = fs::path(filePath);
        return localPath.parent_path().string();
    }
    bool getFileContent(const std::string &filePath, std::string &store) {
        std::ifstream file(filePath); // Open the file for reading

        // Check if the file opened successfully
        if (!file.is_open()) {
            return false;
        }

        // Read the entire file
        std::stringstream buffer;
        buffer << file.rdbuf();
        // Pass the file contents
        store = buffer.str();

        // Success
        return true;
    }
}
