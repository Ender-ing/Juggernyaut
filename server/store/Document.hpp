/**
 * @brief
 * Manage document contents
**/

#pragma once

// Common headers
#include "../../core/common/headers.hpp"

// Basic C++ headers
#include <functional>

// lsp-framework
#include "../lspFramework.hpp"

namespace Store {
    // Listen for syntax-related errors
    class Document {
        public:
            using Event = std::function<void(Document&)>;
        private:
            std::string rawContent = "ERROR: FILE NOT UPDATED!";
            bool isInEditor = false;
            bool isImported = false;

        public:
            const std::string uri;
            Event onRawContentChange;

            // Constructors
            Document() = default;
            Document(const std::string &docUri)
                : uri(std::move(docUri)) {}
            Document(const lsp::DocumentUri &docUri)
                : uri(std::string(docUri.path())) {}
            Document(const std::string &docUri, const bool isOpened)
                : isInEditor(isOpened), uri(std::move(docUri)) {}
            Document(const lsp::DocumentUri &docUri, const bool isOpened)
                : isInEditor(isOpened), uri(std::string(docUri.path())) {}

            void setIsInEditor(bool state) ;
            const bool& getIsInEditor() ;
            void setIsImported(bool state) ;
            const bool& getIsImported() ;
            void setRawContent(std::string rawText) ;
            const std::string& getRawContent() ;
    };
}
