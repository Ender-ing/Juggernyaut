/**
 * @brief
 * Manage document objects and content updates
**/

#include "DocumentStore.hpp"

#include "../../core/common/files.hpp"

// Capabilities
#include "../capabilities/basic.hpp"
#include "../capabilities/semantics/diagnostics.hpp"

// lsp-framework
#include "../lspFramework.hpp"

#include "Document.hpp"

namespace Store {
    void DocumentStore::addDocument(const std::string &uri) {
        Document doc = Document(uri);

        // Get file contents
        std::string rawText;
        bool success = Common::Files::getFileContent(uri, rawText);
        if(!success) {
            rawText = "TEMPORARY:ERROR: COULDN'T FETCH THE FILE!"; // TO-DO: THROW A PROPER ERROR...
        }

        doc.setRawContent(rawText);
        doc.setIsInEditor(false);

        // Events
        doc.onRawContentChange = [](Document document){
            Capabilities::Semantics::validateDocumentSyntax(*Capabilities::handler, document);
        };

        this->documents.insert({uri, doc});
    }
    const Document* DocumentStore::getDocument(const std::string &uri) const {
        auto doc = this->documents.find(uri);
        if (doc != this->documents.end()) {
            return &(doc->second);
        } else {
            return nullptr;
        }
    }
    void DocumentStore::deleteDocument(const std::string &uri) {
        this->documents.erase(uri);
    }
    void DocumentStore::deleteDocument(Document &doc) {
        this->documents.erase(doc.uri);
    }
    void DocumentStore::initDocument(const std::string &uri) {
        auto doc = this->documents.find(uri);
        if (doc == this->documents.end()) {
            this->addDocument(uri);
        }
    }
}
