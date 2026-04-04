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
            : store(srcStore), uri(srcUri) {
            this->id = ++(this->store->lastID);
        }
        SourceId Source::getId() {
            return this->id;
        }


        // Tracking
        void Source::invalidateRawContent() {
            this->shouldUpdateRawContent = true;
            this->shouldUpdateAST = true;
        }
        void Source::invalidateAST() {
            this->shouldUpdateAST = true;
        }
        const bool Source::getUpdateAST() {
            return this->shouldUpdateAST;
        }

        // Dependency tracking
        void Source::addSourceDependency(SourceId dep) {
            std::vector<SourceId> &srcs = this->neededSources;
            if (std::ranges::find(srcs, dep) == srcs.end()) {
                srcs.push_back(dep);
            }
        }
        void Source::removeSourceDependency(SourceId dep) {
            std::vector<SourceId> &srcs = this->neededSources;
            Common::Utility::fastVectorRemove(srcs, dep);
        }
        void Source::resetSourceDependencies() {
            std::vector<SourceId> &srcs = this->neededSources;
            srcs.clear();
            srcs.shrink_to_fit();
        }
        void Source::visitDependencies(DependencyCall depCall) {
            std::vector<SourceId> deps = this->neededSources;
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

        void Source::setIsEntryPoint(const bool state) {
            this->isEntryPoint = state;
        }

        // Diagnostics
        void Source::addParserDiagnostic(Diagnostics::Diagnostic diag) {
            std::vector<Diagnostics::Diagnostic> &diags = this->parserDiagnostics;

            diags.push_back(diag);
        }
        void Source::resetParserDiagnostics() {
            std::vector<Diagnostics::Diagnostic> &diags = this->parserDiagnostics;

            diags.clear();
            diags.shrink_to_fit();
        }
        void Source::visitParserDiagnostics(DiagnosticCall call) {
            std::vector<Diagnostics::Diagnostic> &diags = this->parserDiagnostics;

            for (auto &diag : diags) {
                call(diag);
            }
        }
    }
}
