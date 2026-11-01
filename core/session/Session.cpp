/**
 * @brief 
 * Front-End Session
**/

#include "Session.hpp"

namespace Session {
    // (Inner) Management
    void Session::emitDiagnostic(Diagnostic &diag) {
        // Update internal stats
        if (diag.severity == Diagnostics::Severity::Error ||
            (this->env->build.flags.treatWarningsAsErrors && diag.severity == Diagnostics::Severity::Warning)) {
            this->hasErrors = true;

            if (diag.file != nullptr) {
                auto it = this->units.find(diag.file->id);
                if (it != this->units.end()) {
                    // Check the type of the error
                    const unsigned int phaseCode = std::abs(diag.code) / 100'000;
                    switch (phaseCode) {
                    case 1: // Lexer
                    case 2: // Parser
                        it->second.hasSyntaxErrors = true;
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        // Trigger relevent events
        if (this->hooks.onDiagnostic != nullptr) {
            if (this->env->build.flags.treatWarningsAsErrors &&
                    diag.severity == Diagnostics::Severity::Warning) {
                diag.severity = Diagnostics::Severity::Error;
            }
            if (this->env->build.flags.ignoreWarnings &&
                diag.severity == Diagnostics::Severity::Warning) {
                return;
            }
            this->hooks.onDiagnostic(diag);
        }
    }

    // Source & Dependency Registration (Called by JugDriver)
    void Session::addEntryPoint(const File* file) {
        if (file == nullptr) {
            return;
        }

        // Only add to entryPoints if not already present
        if (std::ranges::find(this->entryPoints, file->id) == this->entryPoints.end()) {
            this->entryPoints.push_back(file->id);
        }

        // Update entry status
        auto [it, inserted] = this->units.try_emplace(file->id, CompilationUnitState{.file = file});
        it->second.isEntryPoint = true;
    }
    void Session::refreshEntryPoints() {
        // Reset entry points
        this->entryPoints.clear();

        // Check for environment entry points
        if (this->configs.inheritEnvironmentBuildInputs && this->env != nullptr) {
            for (const std::string &uri : this->env->build.inputPaths) {
                const File* file = this->vfs->readFile(uri);

                addEntryPoint(file);
            }
        }
    }

    // Pipeline
    void Session::run() {
        this->isRunning = true;

        // Handle exit logic
        struct RunCleanup {
            Session *self;
            ~RunCleanup() {
                // Trigger relevent events
                if (self->hooks.onRunFinished != nullptr) {
                    self->hooks.onRunFinished(self);
                }
            }
        } cleanup{this};

        // Refresh dependency graph
        this->refreshEntryPoints();
        // ....

        // Temporary dummy code
        // REMOVE LATER!
        for (SourceId id: this->entryPoints) {
            const File *file = this->vfs->readFile(id);

            Diagnostic diag = {.file = file};
            diag.range.end.character = 3;
            diag.message = "This resource has been loaded! (The parser is being implemented)";
            diag.severity = Diagnostics::Severity::Info;
            this->emitDiagnostic(diag);
        }

        this->isRunning = false;
    }
}
