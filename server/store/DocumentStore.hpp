/**
 * @brief
 * Manage document objects and content updates
**/

#pragma once

// Common headers
#include "../../core/common/headers.hpp"

// Basic C++ headers
#include <unordered_map>

#include "Document.hpp"

namespace Store {
    class DocumentStore {
        private:
            std::unordered_map<std::string, Document> documents;
            void addDocument(const std::string &uri, bool fetchContent) ;

        public:
            Document* getDocument(const std::string &uri) ;
            const Document* getDocument(const std::string &uri) const ;
            void deleteDocument(const std::string uri) ;
            // Triggers addDocument when needed!
            void initDocument(const std::string &uri) ;
    };
}
