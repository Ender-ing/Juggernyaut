/**
 * @brief 
 * "Source code" instance
**/

#include "Source.hpp"

#include "common/utility.hpp"

// Basic C++ libraries
#include <algorithm>

#include "SourceStore.hpp"

namespace Data {
    namespace Store {
        namespace Internal {
            static void resetDiagStats(DiagnosticsSection &diags) {
                diags.errors = 0;
                diags.warnings = 0;
                diags.info = 0;
                diags.hints = 0;
            }
            static void updateDiagStats(DiagnosticsSection &diags, Diagnostics::Severity &severity) {
                switch (severity) {
                case Diagnostics::Severity::Error:
                    (diags.errors)++;
                    break;
                case Diagnostics::Severity::Warning:
                    (diags.warnings)++;
                    break;
                case Diagnostics::Severity::Hint:
                    (diags.hints)++;
                    break;
                case Diagnostics::Severity::Info: // Default
                default:
                    (diags.info)++;
                    break;
                }
            }
        }

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
        void Source::preventUpdateAST() {
            this->shouldUpdateAST = false;
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
            // MISSING: ERROR REPORTING
            srcStore->_getRawFile(this->uri, this->rawContent);
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
            DiagnosticsBatch &diags = this->diagnostics;

            diags.parser.diagnostics.push_back(diag);

            Internal::updateDiagStats(diags.parser, diag.severity);
        }
        void Source::resetParserDiagnostics() {
            DiagnosticsBatch &diags = this->diagnostics;

            diags.parser.diagnostics.clear();
            diags.parser.diagnostics.shrink_to_fit();

            Internal::resetDiagStats(diags.parser);
        }
        void Source::visitParserDiagnostics(DiagnosticCall call) {
            DiagnosticsBatch &diags = this->diagnostics;

            for (auto &diag : diags.parser.diagnostics) {
                call(diag);
            }
        }
    }
}
