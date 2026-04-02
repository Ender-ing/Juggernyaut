/**
 * @brief 
 * "Source code" instance
**/

#include "Source.hpp"

namespace Data {
    namespace Store {
        // Tracking
        void Source::requestRawUpdate() {
            this->shouldUpdateRawContent = true;
        }

        // Raw content
        void Source::fetchRawContent() {
            SourceStore *srcStore = this->store;
            this->rawContent = std::move(srcStore->onFileRawRequest(this->uri));
        }
        const std::string& Source::getRawContent() {
            if (this->shouldUpdateRawContent) {
                this->fetchRawContent();
            }
            return this->rawContent;
        }
    }
}
