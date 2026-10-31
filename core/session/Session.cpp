/**
 * @brief 
 * Front-End Session
**/

#include "Session.hpp"

namespace Session {
    // (Inner) Management
    void Session::emitDiagnostic(const Diagnostic &diag) {
        if (this->hooks.onDiagnostic != nullptr){
            this->hooks.onDiagnostic(diag);
        }
    }

    // Source & Dependency Registration (Called by JugDriver)
    void Session::addEntryPoint(const File* file) {
        CompilationUnitState unit = {file = file};

        // Update entry status
        this->entryPoints.push_back(file->id);
        unit.isEntryPoint = true;

        // Move unit
        this->units[file->id] = std::move(unit);
    }

    // Pipeline
    void Session::run() {
        // Refresh dependency graph
        this->refreshEntryPoints();
        // ....

        // ...
    }
}
