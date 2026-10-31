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
    void Session::refreshEntryPoints() {
        // Reset entry points
        this->entryPoints.clear();

        // Check for environment entry points
        if (this->configs.inheritEnvironmentBuildInputs) {
            for (const std::string &uri : this->env->build.inputPaths) {
                const File* file = this->vfs->readFile(uri);

                addEntryPoint(file);
            }
        }
    }

    // Pipeline
    void Session::run() {
        // Refresh dependency graph
        this->refreshEntryPoints();
        // ....

        // ...
    }
}
