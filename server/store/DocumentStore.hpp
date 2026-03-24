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
    extern const std::nullopt_t NO_DOC;
    class DocumentStore {
        private:
            std::unordered_map<std::string, Document> documents;
            virtual void addDocument(const std::string &uri) ;

        public:
            virtual const Document* getDocument(const std::string &uri) const ;
            // Triggers addDocument when needed!
            virtual void initDocument(const std::string &uri) ;
    };
}
