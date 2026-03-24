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

            virtual void setIsInEditor(bool state) ;
            virtual const bool& getIsInEditor() ;
            virtual void setRawContent(std::string rawText) ;
            virtual const std::string& getRawContent() ;
    };
}
