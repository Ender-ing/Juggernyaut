/**
 * @brief
 * File-related functions
**/

#include "files.hpp"

// Basic C++ headers
#include <fstream>

namespace fs = std::filesystem;

namespace Store {
    // Internal functions
    fs::path normalizePath(fs::path &path) {
        return fs::weakly_canonical(path);
    }
    std::string normalizePath(const std::string &path) {
        fs::path fsPath(path);
        return normalizePath(fsPath).string();
    }
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
    std::string getParentPath(const std::string &path) {
        fs::path localPath = fs::path(path);
        return localPath.parent_path().string();
    }
    // Get the current process executable directory
    bool getExecutableDir(std::string &store) {
        #ifdef _WIN32
            // Windows logic
            wchar_t buffer[MAX_PATH] = { 0 };
            if (GetModuleFileNameW(NULL, buffer, MAX_PATH) == 0) {
                return false;
            }
            std::wstring wExecutablePath(buffer);

            // Get executable path
            fs::path executablePath(wExecutablePath);
        #elif __APPLE__
            // macOS logic
            char buffer[PATH_MAX] = {0};
            uint32_t bufsize = PATH_MAX;
            if (_NSGetExecutablePath(buffer, &bufsize) != 0) {
                return false;
            }
            // Resolve any symlinks and get the canonical path
            char realPath[PATH_MAX];
            if (realpath(buffer, realPath) == NULL) {
                return false;
            }

            // Get executable path
            fs::path executablePath((std::string) realPath);
        #else
            // Unix-like logic
            char buffer[PATH_MAX] = { 0 };
            if (readlink("/proc/self/exe", buffer, PATH_MAX) == -1) {
                return false;
            }

            // Get executable path
            fs::path executablePath((std::string) buffer);
        #endif
    
        // Remove filename
        executablePath.remove_filename();

        // Pass value
        store = normalizePath(executablePath).string();
        // Success
        return true;
    }
}
