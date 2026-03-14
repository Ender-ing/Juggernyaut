/**
 * @brief
 * File-related functions
**/

#include "files.hpp"

// Basic C++ headers
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace Common {
    namespace Files {
        // Internal functions
        static fs::path normalizePath(fs::path &path) {
            return path.append(".").lexically_normal();
        }
        // Check if a file is accessible
        bool isFileAccessible(const std::string &filePath) {
            // Check if the file is open!
            std::ifstream file(filePath);
            return file.is_open(); // FIle will be closed when out of scope
        }
        // Check if the file is of a valid format
        bool isFileValid(const std::string &filePath) {
            // Check file extension
            fs::path filePathObj = filePath;
            std::string extension = filePathObj.extension().string();
            return (extension == ".jug");
            /** @brief INCOMPLETE: Add a file data checker! **/
        }
        // Get the contents of a file
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
        // Get the parent folder's path
        std::string getParentPath(const std::string &path, int depth) {
            // Normalise the path
            fs::path localPath = fs::path(path).lexically_normal();
            // Get the parent folder's path string
            for (int i = 0; i < depth + 1; i++) {
                localPath = localPath.parent_path();
            }
            return normalizePath(localPath).string();
        }
        std::string getParentPath(const char *path, int depth) {
            return getParentPath((std::string) path, depth);
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
}
