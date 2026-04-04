/**
 * @brief
 * Manage file objects
**/

#include "files.hpp"

#include "FileStore.hpp"

#include "../console/console.hpp"

namespace Store {
    std::string FileStore::onFileRawRequest(const std::string &uri) {
        // Check if the file can be opened!
        if (!isFileAccessible(uri)) {
            REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "file \"", uri,
                "\" is non-existent or inaccessible", Console::END_REPORT);

            return std::string("");
        }

        // Check if the file can be opened!
        if (!isFileValid(uri)) {
            REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "file \"", uri,
                "\" is corrupted or of an invalid type! (Must use a valid .jug file)", Console::END_REPORT);

            return std::string("");
        }

        // Get file content
        std::string content;
        if (getFileContent(uri, content)) {
            return content;
        } else {
            REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "couldn't open file: ", uri, Console::END_REPORT);
            return std::string("");
        }
    }
}
