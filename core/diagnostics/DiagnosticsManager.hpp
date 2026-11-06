/**
 * @brief
 * Handle diagnostic grouping (For LSP-like scenarios)
**/

#pragma once

#include "common/headers.hpp"

// Diagnostics
#include "Diagnostic.hpp"

// Driver (ONLY THE HOOKS!)
#include "../driver/Hooks.hpp"

namespace Diagnostics {
    // Hooks
    using FrontEndHooks = Driver::Hooks;
    /*struct ManagerHooks {
        // Front-End Driver (General)
        Diagnostics::Event *onDriverDiagnostic = nullptr;
        GenericEvent *onDriverRunFinished = nullptr;

        // Front-End Driver (Environments)
        FileEvent *onEnvironmentLoad = nullptr;
        FileEvent *onEnvironmentChange = nullptr;

        // Front-End Driver (Units/Sessions)
        FileEvent *onLexerContextEnd = nullptr;
        FileEvent *onParserContextEnd = nullptr;
        FileEvent *onContextEnd = nullptr; // If this is not set, this you'd likely needs to manually flush the diagnostics when you are done!

        // Back-End Compiler
        // Not yet!
    };*/

    // NOTE: Final stage flusing logic can change as more compilation phases are added!
    // WARNING: Your DiagnosticsManager should be accessed by one thread at a time!
    template <typename TargetDiag = Diagnostic>
    class DiagnosticsManager {
        public:
            using ConverterFn = std::function<TargetDiag(const Diagnostic&)>; // This is done to prevent data duplication in memory as much as possible when dealing with different frameworks!
            using FileFlushCallback = std::function<void(const std::string &uri, const std::vector<TargetDiag> &diagnostics)>;
            using ComponentFlushCallback = std::function<void(const TargetDiag &diagnostic)>;
            struct ManagerCallbacks {
                FileFlushCallback onFileFlush = nullptr;
                ComponentFlushCallback onComponentFlush = nullptr;
            };

        private:
            std::mutex hookMtx;
            std::unordered_map<std::string, std::vector<TargetDiag>> fileDiagnostics;
            std::unordered_map<std::string, bool> isFileDetached;
            std::vector<TargetDiag> componentDiagnostics; // diag.file == nullptr
            const ConverterFn converter;
            const ManagerCallbacks callbacks;

            // Internal
            // Record a diagnostic (copies URI string immediately while inside the event)
            void push(const Diagnostic &diag) {
                if (this->converter == nullptr) {
                    return;
                }
                TargetDiag convertedDiag = this->converter(diag);

                std::lock_guard<std::mutex> lock(this->hookMtx);

                if (diag.file != nullptr) {
                    // Copy relevent data immediately!
                    const std::string uri = diag.file->canonical;
                    this->fileDiagnostics[uri].push_back(convertedDiag);
                } else {
                    this->componentDiagnostics.push_back(convertedDiag);
                }
            }

            // Flush a single file by canonical URI:
            // - clearAfter == false (for Intermediate stages)
            // - clearAfter == true (for the Final stage)
            void flushFile(const std::string &uri, bool clearAfter = false) {
                if (this->callbacks.onFileFlush == nullptr) {
                    return;
                }
                std::lock_guard<std::mutex> lock(this->hookMtx);

                auto it = this->fileDiagnostics.find(uri);
                if (it != this->fileDiagnostics.end()) {
                    this->callbacks.onFileFlush(uri, it->second);

                    if (clearAfter) {
                        this->fileDiagnostics.erase(it); // Free memory after final stage!
                    }
                } else {
                    // The diagnostics list isn't present!

                    // File finished its final stage with 0 diagnostics: send empty `[]` to clear any old squiggles in the editor!
                    static const std::vector<TargetDiag> emptyList;
                    this->callbacks.onFileFlush(uri, emptyList);
                }
            }
            void emptyFlush(const std::string &uri) {
                if (this->callbacks.onFileFlush == nullptr) {
                    return;
                }
                std::lock_guard<std::mutex> lock(this->hookMtx);

                static const std::vector<TargetDiag> emptyList;
                this->callbacks.onFileFlush(uri, emptyList);
            }

            // Internal (Diagnostic dispatch logic)
            void setupDiagnosticDispatch(FrontEndHooks &frontEnd) {
                // Handle incoming Diagnostics
                frontEnd.driver.onDiagnostic = [this](const Diagnostic &diag) {
                    this->push(diag);
                };

                // Helper functions
                static FileEvent flushAndKeep = [this](const Data::Host::File *file) {
                    if (file != nullptr) {
                        this->flushFile(file->canonical, false);
                    }
                };
                static FileEvent flushAndDiscard = [this](const Data::Host::File *file) {
                    if (file != nullptr) {
                        this->flushFile(file->canonical, true);
                    }
                };

                // Handle Diagnostics Retention logic
                // [jug.toml]
                frontEnd.driver.onEnvironmentLoad = flushAndDiscard;
                frontEnd.driver.onEnvironmentChange = flushAndDiscard;
                // [*.jug]
                frontEnd.unitSources.onContextStart = [this](const File *file) {
                    // Update state
                    if (this->isFileDetached[file->canonical]) {
                        this->isFileDetached[file->canonical] = false;
                    }

                    // Reset diagnostics (unchanged client state!)
                    this->fileDiagnostics[file->canonical].clear();
                };
                //frontEnd.unitSources.onLexerContextEnd = flushAndKeep; // Too much?
                frontEnd.unitSources.onParserContextEnd = flushAndKeep;
                frontEnd.unitSources.onContextEnd = flushAndKeep; // TMP
                frontEnd.unitSources.onContextSkip = [this](const File *file) {
                    // Refresh detached files (in case no edits were made!)
                    if (this->isFileDetached[file->canonical]) {
                        this->isFileDetached[file->canonical] = false;
                        flushAndKeep(file);
                    }
                };
                frontEnd.unitSources.onContextDetached = [this](const File *file) {
                    this->isFileDetached[file->canonical] = true;
                    this->emptyFlush(file->canonical);
                };
                /*frontEnd.driver.onRunFinished = [this]() { // This runs before 'onContextDetached'!
                    // this->manualFlushAll(false);
                };*/
            }

        public:
            // Constructor: overtakes `diagnosticHandle` and converts incoming diagnostics
            DiagnosticsManager(FrontEndHooks &hooks, ConverterFn convertFn, ManagerCallbacks managerCallbacks)
                : converter(std::move(convertFn)), callbacks(std::move(managerCallbacks)) {
                setupDiagnosticDispatch(hooks);
            }
            // Overload when TargetDiag == Diagnostics::Diagnostic (no converter needed)
            DiagnosticsManager(FrontEndHooks &hooks, ManagerCallbacks managerCallbacks)
                requires std::same_as<TargetDiag, Diagnostic>
                : DiagnosticsManager(
                    hooks,
                    [](const Diagnostic &d) -> Diagnostic { return d; },
                    std::move(managerCallbacks)
                ) {}
            ~DiagnosticsManager() = default;

            // Non-copyable (since the bound lambda captures `this`)
            DiagnosticsManager(const DiagnosticsManager&) = delete;
            DiagnosticsManager& operator=(const DiagnosticsManager&) = delete;
            DiagnosticsManager(DiagnosticsManager&&) = delete;
            DiagnosticsManager& operator=(DiagnosticsManager&&) = delete;

            // Prepare for a new run: clear vectors, but KEEP keys that had errors on the last run
            // so if they have 0 errors on this run, `flush()` sends an empty `[]` to clear the editor!
            void clearAllDiagnostics() {
                std::lock_guard<std::mutex> lock(this->hookMtx);

                for (auto &[uri, list] : this->fileDiagnostics) {
                    list.clear();
                }
                this->componentDiagnostics.clear();
            }

            // Flush all grouped file diagnostics and component diagnostics
            void manualFlushAll(bool clearAll = true) {
                std::lock_guard<std::mutex> lock(this->hookMtx);

                if (this->callbacks.onFileFlush != nullptr) {
                    for (const auto &[uri, list] : this->fileDiagnostics) {
                        this->callbacks.onFileFlush(uri, list);
                    }
                }
                if (this->callbacks.onComponentFlush != nullptr) {
                    for (const auto &compDiag : this->componentDiagnostics) {
                        this->callbacks.onComponentFlush(compDiag);
                    }
                }

                if (clearAll) {
                    this->fileDiagnostics.clear();
                    this->componentDiagnostics.clear();
                }
            }
    };
}
