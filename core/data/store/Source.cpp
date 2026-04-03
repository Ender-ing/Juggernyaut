/**
 * @brief 
 * "Source code" instance
**/

#include "Source.hpp"

#include "../../common/utility.hpp"

// Basic C++ libraries
#include <algorithm>

#include "SourceStore.hpp"

namespace Data {
    namespace Store {
        // Init
        Source::Source(std::string srcUri, SourceStore *srcStore)
            : uri(srcUri), store(srcStore) {
            this->id = ++(this->store->lastID);
        }
        SourceID Source::getID() {
            return this->id;
        }


        // Tracking
        void Source::requestRawUpdate() {
            this->shouldUpdateRawContent = true;
            this->shouldUpdateAST = true;
        }

        // Dependency tracking
        void Source::addSourceDependency(SourceID dep) {
            std::vector<SourceID> &srcs = this->neededSources;
            if (std::ranges::find(srcs, dep) == srcs.end()) {
                srcs.push_back(dep);
            }
        }
        void Source::removeSourceDependency(SourceID dep) {
            std::vector<SourceID> &srcs = this->neededSources;
            Common::Utility::fastVectorRemove(srcs, dep);
        }
        void Source::resetSourceDependencies() {
            std::vector<SourceID> &srcs = this->neededSources;
            srcs.clear();
            srcs.shrink_to_fit();
        }
        void Source::visitDependencies(DependencyCall depCall) {
            std::vector<SourceID> deps = this->neededSources;
            for (auto dep : deps) {
                if (depCall != nullptr) {
                    depCall(dep);
                }
            }
        }

        // Raw content
        void Source::fetchRawContent() {
            SourceStore *srcStore = this->store;
            this->rawContent = srcStore->onFileRawRequest(this->uri);
        }
        const std::string& Source::getRawContent() {
            if (this->shouldUpdateRawContent) {
                this->fetchRawContent();
                this->shouldUpdateRawContent = false;
            }
            return this->rawContent;
        }

        // AST data
        void Source::setUpdateAST(const bool state) {
            this->shouldUpdateAST = state;
        }
        const bool Source::getUpdateAST() {
            return this->shouldUpdateAST;
        }

        void Source::setIsEntryPoint(const bool state) {
            this->isEntryPoint = state;
        }
    }
}
