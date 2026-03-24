/**
 * @brief
 * Manage document contents
**/

#include "Document.hpp"

namespace Store {
    void Document::setIsInEditor(bool state) {
        this->isInEditor = state;
    }
    const bool& Document::getIsInEditor() {
        return this->isInEditor;
    }
    void Document::setRawContent(std::string rawText) {
        this->rawContent= rawText;
        if (this->onRawContentChange != nullptr) {
            Document::Event callback = this->onRawContentChange;
            callback(*this);
        }
    }
    const std::string& Document::getRawContent() {
        return this->rawContent;
    }
}
