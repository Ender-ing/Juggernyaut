/// @file
/// @ingroup CoreDiagnostics
/// @brief Defines #Diagnostics::DiagnosticsManager for grouping, converting, and flushing diagnostics by file or component.
///
/// @see Diagnostics::Diagnostic
/// @see Driver::Hooks

#pragma once

#include "common/headers.hpp"

// Diagnostics
#include "Diagnostic.hpp"

// Driver (ONLY THE HOOKS!)
#include "../driver/Hooks.hpp"

namespace Diagnostics {

    /// @addtogroup CoreDiagnostics
    /// @{

    // Hooks
    /// @brief Alias for the front-end driver hook table (`Driver::Hooks`) wired by #DiagnosticsManager.
    /// @see DiagnosticsManager
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
    /// @brief Aggregates, converts, and flushes per-file and component-level #Diagnostic instances across compilation stages.
    ///
    /// `#DiagnosticsManager` attaches itself to a #FrontEndHooks instance, converting incoming
    /// #Diagnostic reports into `TargetDiag` immediately (to avoid duplicating memory when targeting
    /// external frameworks such as `lsp-framework`) and dispatching grouped batches through
    /// #ManagerCallbacks when files complete stages or detach.
    ///
    /// @note Final stage flushing logic can change as more compilation phases are added.
    /// @warning Internal state is synchronized via a mutex during hook callbacks, but external control methods should be invoked by one thread at a time.
    ///
    /// @tparam TargetDiag Destination diagnostic representation type (defaults to #Diagnostics::Diagnostic).
    ///
    /// @see Diagnostic
    /// @see ManagerCallbacks
    /// @see FrontEndHooks
    template <typename TargetDiag = Diagnostic>
    class DiagnosticsManager {
        public:
            /// @brief Function signature that converts a core #Diagnostic into a `TargetDiag` instance.
            /// @see Diagnostic
            using ConverterFn = std::function<TargetDiag(const Diagnostic&)>; // This is done to prevent data duplication in memory as much as possible when dealing with different frameworks!

            /// @brief Callback signature invoked when all accumulated diagnostics for a single file URI are flushed.
            /// @see ManagerCallbacks
            using FileFlushCallback = std::function<void(const std::string &uri, const std::vector<TargetDiag> &diagnostics)>;

            /// @brief Callback signature invoked when a non-file component diagnostic (`diag.file == nullptr`) is flushed.
            /// @see ManagerCallbacks
            using ComponentFlushCallback = std::function<void(const TargetDiag &diagnostic)>;

            /// @brief Sink callbacks invoked by #DiagnosticsManager when flushing file-scoped or component-scoped diagnostics.
            ///
            /// @see DiagnosticsManager
            /// @see FileFlushCallback
            /// @see ComponentFlushCallback
            struct ManagerCallbacks {
                FileFlushCallback onFileFlush = nullptr;           ///< @brief Invoked with a canonical file URI and its current list of `TargetDiag` items.
                ComponentFlushCallback onComponentFlush = nullptr; ///< @brief Invoked for each component-wide `TargetDiag` not tied to a source file.
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
            /// @brief Constructs a #DiagnosticsManager with a custom #ConverterFn and binds to @p hooks.
            ///
            /// @param[in,out] hooks            Front-end driver hooks whose diagnostic and context callbacks will be bound to this manager.
            /// @param[in]     convertFn        Function used to convert each incoming #Diagnostic into `TargetDiag`.
            /// @param[in]     managerCallbacks Flush callbacks invoked when file or component diagnostics are ready.
            ///
            /// @see ManagerCallbacks
            /// @see ConverterFn
            DiagnosticsManager(FrontEndHooks &hooks, ConverterFn convertFn, ManagerCallbacks managerCallbacks)
                : converter(std::move(convertFn)), callbacks(std::move(managerCallbacks)) {
                setupDiagnosticDispatch(hooks);
            }

            // Overload when TargetDiag == Diagnostics::Diagnostic (no converter needed)
            /// @brief Constructs a #DiagnosticsManager using an identity converter when `TargetDiag` is #Diagnostics::Diagnostic.
            ///
            /// @param[in,out] hooks            Front-end driver hooks whose diagnostic and context callbacks will be bound to this manager.
            /// @param[in]     managerCallbacks Flush callbacks invoked when file or component diagnostics are ready.
            ///
            /// @see ManagerCallbacks
            DiagnosticsManager(FrontEndHooks &hooks, ManagerCallbacks managerCallbacks)
                requires std::same_as<TargetDiag, Diagnostic>
                : DiagnosticsManager(
                    hooks,
                    [](const Diagnostic &d) -> Diagnostic { return d; },
                    std::move(managerCallbacks)
                ) {}

            /// @brief Destroys the #DiagnosticsManager instance.
            ~DiagnosticsManager() = default;

            // Non-copyable (since the bound lambda captures `this`)
            /// @brief Deleted copy constructor (bound hook lambdas capture `this`).
            DiagnosticsManager(const DiagnosticsManager&) = delete;

            /// @brief Deleted copy-assignment operator (bound hook lambdas capture `this`).
            DiagnosticsManager& operator=(const DiagnosticsManager&) = delete;

            /// @brief Deleted move constructor (bound hook lambdas capture `this`).
            DiagnosticsManager(DiagnosticsManager&&) = delete;

            /// @brief Deleted move-assignment operator (bound hook lambdas capture `this`).
            DiagnosticsManager& operator=(DiagnosticsManager&&) = delete;

            // Prepare for a new run: clear vectors, but KEEP keys that had errors on the last run
            // so if they have 0 errors on this run, `flush()` sends an empty `[]` to clear the editor!
            /// @brief Clears accumulated diagnostic vectors for a new run while retaining file URI keys.
            ///
            /// Preserving existing keys in the map ensures that if a file had errors on the previous run
            /// and finishes the next run with 0 errors, flushing sends an empty `[]` batch to clear stale editor squiggles.
            ///
            /// @see manualFlushAll
            void clearAllDiagnostics() {
                std::lock_guard<std::mutex> lock(this->hookMtx);

                for (auto &[uri, list] : this->fileDiagnostics) {
                    list.clear();
                }
                this->componentDiagnostics.clear();
            }

            // Flush all grouped file diagnostics and component diagnostics
            /// @brief Immediately invokes #ManagerCallbacks for all buffered file and component diagnostics.
            ///
            /// @param[in] clearAll If `true` (default), erases all buffered file and component diagnostics after flushing.
            ///
            /// @see clearAllDiagnostics
            /// @see ManagerCallbacks
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

    /// @}
}
