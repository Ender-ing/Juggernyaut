/**
 * @brief 
 * Front-End Session
**/

#include "Session.hpp"
#include "common/utility.hpp"

#include <unordered_set>

namespace Session {
    // (Internal) Management
    void Session::emitDiagnostic(Diagnostic &diag) {
        if (this->env != nullptr) {
            // Upgrade warnings to errors!
            if (this->env->build.flags.treatWarningsAsErrors &&
                diag.severity == Diagnostics::Severity::Warning) {
                diag.severity = Diagnostics::Severity::Error;
            }
            // Ignore warnings
            if (this->env->build.flags.ignoreWarnings &&
                diag.severity == Diagnostics::Severity::Warning) {
                return;
            }
        }

        // Update internal stats
        if (diag.severity == Diagnostics::Severity::Error) {
            this->hasErrors = true;

            if (diag.file != nullptr) {
                auto it = this->units.find(diag.file->id);
                if (it != this->units.end()) {
                    // Check the type of the error
                    const int phaseCode = std::abs(diag.code) / 100'000;
                    switch (phaseCode) {
                    case 1: // Lexer
                    case 2: // Parser
                    case 3: // Transformer
                        it->second.hasSyntaxErrors = true;
                        break;
                    case 4: // Semantic
                        it->second.hasSemanticErrors = true;
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        // Trigger relevant events
        if (this->hooks.onDiagnostic != nullptr) {
            this->hooks.onDiagnostic(diag);
        }
    }
    void Session::checkDetachedSources() {
        if (this->isRunning || this->hooks.onContextDetached == nullptr) {
            return;
        }

        // Look for detached sources!
        for (const auto &[id, state] : this->units) {
            if (!state.isReachable && !state.isEntryPoint) {
                this->hooks.onContextDetached(state.file);
            }
        }
    }

    // Source & Dependency Registration (Called by JugDriver)
    void Session::addEntryPoint(const File *file) {
        if (file == nullptr) {
            return;
        }

        // Only add to entryPoints if not already present
        if (std::ranges::find(this->entryPoints, file->id) == this->entryPoints.end()) {
            this->entryPoints.push_back(file->id);
        }

        // Update entry status
        auto [it, inserted] = this->units.try_emplace(file->id, CompilationUnitState{.file = file});
        it->second.file = file;
        it->second.isEntryPoint = true;
    }
    void Session::refreshEntryPoints() {
        // Reset entry points
        for (auto &[id, unit] : this->units) {
            unit.isEntryPoint = false;
            unit.isReachable = false;
        }
        this->entryPoints.clear();

        if (this->configs.inheritEnvironmentBuildInputs && this->env != nullptr && this->vfs != nullptr) {
            for (const std::string &uri : this->env->build.inputPaths) {
                const File *file = this->vfs->readFile(uri);
                this->addEntryPoint(file);
            }
        }
    }

    // Parser Stage Helpers
    Parser::Parser Session::createConfiguredParser() {
        Parser::Configs parserConfigs{
            .terminateAfterLexer = (this->configs.terminateAfter == Stage::Lexer)
        };

        Parser::Hooks parserHooks{
            .onContextSkip = this->hooks.onContextSkip,
            .onContextStart = this->hooks.onContextStart,
            .onLexerContextEnd = this->hooks.onLexerContextEnd,
            .onParserContextEnd = this->hooks.onParserContextEnd,
            .onContextEnd = this->hooks.onContextEnd,
            .onDiagnostic = [this](Diagnostic &diag) {
                this->emitDiagnostic(diag);
            }
        };

        return Parser::Parser(std::move(parserConfigs), std::move(parserHooks), this->env);
    }
    void Session::resolveUnitImports(const File *sourceFile, CompilationUnitState &unitState,
        const std::vector<Parser::ImportRequest> &imports, std::vector<SourceId> &worklist) {
        // Clear old outgoing dependency edges
        for (SourceId oldDepId : unitState.dependencies) {
            auto depIt = this->units.find(oldDepId);
            if (depIt != this->units.end()) {
                Common::Utility::fastVectorRemove(depIt->second.dependents, sourceFile->id);
            }
        }
        unitState.dependencies.clear();

        // URIs are relative to the directory of the source file!
        const std::string parentDir = this->vfs->getParentDirectory(sourceFile->canonical);

        for (const Parser::ImportRequest &imp : imports) {
            if (imp.isExternal) {
                const bool hasExternalDeps = this->env != nullptr &&
                    (!this->env->localDeps.empty() || !this->env->onlineDeps.empty());

                Diagnostic diag;
                diag.file = sourceFile;
                diag.range = imp.range;
                if (hasExternalDeps) {
                    diag.code = 1;
                    diag.message = "external resource inclusion is not supported yet.";
                    diag.severity = Diagnostics::Severity::Warning;
                } else {
                    USE_CODE(diag, Error, 600001);
                }
                this->emitDiagnostic(diag);

                continue;
            }

            // Local file import resolution via VFS
            const std::string resolvedUri = this->vfs->join(parentDir, imp.target);
            if (!this->vfs->exists(resolvedUri)) {
                Diagnostic diag = {.file = sourceFile};
                diag.range = imp.range;

                USE_CODE(diag, Error, 600002, resolvedUri, "file does not exist");
                this->emitDiagnostic(diag);
                continue;
            }

            if (!this->vfs->endsWith(resolvedUri, ".jug")) {
                Diagnostic diag = {.file = sourceFile};
                diag.range = imp.range;

                USE_CODE(diag, Error, 600002, resolvedUri, "target is not a '.jug' source file");
                this->emitDiagnostic(diag);

                continue;
            }

            const File *depFile = this->vfs->readFile(resolvedUri);
            if (depFile == nullptr || !(depFile->isValid)) {
                Diagnostic diag = {.file = sourceFile};
                diag.range = imp.range;

                USE_CODE(diag, Error, 600002, imp.target, "unable to read file content");
                this->emitDiagnostic(diag);

                continue;
            }

            // Register dependency unit
            auto [depIt, inserted] = this->units.try_emplace(depFile->id, CompilationUnitState{.file = depFile});

            // Keep relations up-to-date!
            if (!Common::Utility::isInVector(unitState.dependencies, depFile->id)) {
                unitState.dependencies.push_back(depFile->id);
            }
            if (!Common::Utility::isInVector(depIt->second.dependents, sourceFile->id)) {
                depIt->second.dependents.push_back(sourceFile->id);
            }

            worklist.push_back(depFile->id);
        }
    }
    bool Session::runParserStage() {
        if (this->vfs == nullptr) {
            return false;
        }

        Parser::Parser parser = this->createConfiguredParser();
        const Stage targetStage = (this->configs.terminateAfter == Stage::Lexer)
            ? Stage::Lexer
            : Stage::Parser;

        std::vector<SourceId> worklist = this->entryPoints; // Copy entry points!
        std::unordered_set<SourceId> visitedInRun;

        while (!worklist.empty()) {
            const SourceId currentId = worklist.back();
            worklist.pop_back();

            // Check if the source has been processed already!
            if (!visitedInRun.insert(currentId).second) {
                continue;
            }

            // Get the current file
            const File *file = this->vfs->readFile(currentId);
            if (file == nullptr) {
                continue; // Maybe a warning should be issued?
            }

            // Create a unit
            auto [unitIt, inserted] = this->units.try_emplace(currentId, CompilationUnitState{.file = file});
            CompilationUnitState &unit = unitIt->second;
            unit.isReachable = true;

            // Skip parsing if the file revision and stage are already up-to-date
            const bool isUpToDate = (unit.contentRevision == file->version) &&
                                    (unit.completedStage >= targetStage);
            if (isUpToDate) {
                // Trigger relevant events 
                if (this->hooks.onContextSkip != nullptr) {
                    this->hooks.onContextSkip(file);
                }

                // Add dependencies to the work list!
                for (SourceId depId : unit.dependencies) {
                    worklist.push_back(depId);
                }

                continue;
            }

            // Fresh parse for this compilation unit
            unit.hasSyntaxErrors = false;
            unit.contentRevision = file->version;

            Parser::ParseResult result = parser.parse(file);
            unit.hasSyntaxErrors = result.hasSyntaxErrors;
            unit.completedStage = targetStage;

            if (targetStage >= Stage::Parser) {
                this->astTable[currentId] = std::move(result.ast);
                this->resolveUnitImports(file, unit, result.imports, worklist);
            }

            // Trigger relevant events
            if (parser.hooks.onContextEnd != nullptr) {
                parser.hooks.onContextEnd(file);
            }
        }

        return !this->hasErrors;
    }

    // Pipeline
    void Session::run() {
        if (this->isRunning) {
            return;
        }
        this->isRunning = true;
        this->hasErrors = false;

        struct RunCleanup {
            Session *self;
            ~RunCleanup() {
                self->isRunning = false;

                // Trigger relevant events
                /*if (self->hooks.onRunFinished != nullptr) {
                    self->hooks.onRunFinished(self);
                }*/
                // Trigger relevant events
                self->checkDetachedSources();

            }
        } cleanup{this};

        if (this->configs.terminateAfter == Stage::Unprocessed) {
            return;
        }

        // Refresh entry points
        this->refreshEntryPoints();

        // [STAGE 1 & 2] Lexer & Parser
        this->runParserStage();
        if (this->configs.terminateAfter == Stage::Lexer ||
            this->configs.terminateAfter == Stage::Parser) {
            return;
        }

        // [STAGE 3] Transformer
        if (this->configs.terminateAfter == Stage::Transformer) {
            return;
        }

        // [STAGE 4] Semantic
        if (this->configs.terminateAfter == Stage::Semantic) {
            return;
        }

        // [STAGE 5] IaRC
        // ...
    }

    void Session::rejuvenate() {
        if (this->isRunning) {
            return;
        }

        // Prune unreachable compilation units and their data!
        std::erase_if(this->units, [this](const auto &item) {
            const auto &[id, state] = item;
            if (!state.isReachable && !state.isEntryPoint) {
                // Cleanup!
                this->astTable.erase(id);

                return true;
            }

            return false;
        });

        // Parser::Parser::cleanup(); // Dangerous to run here!
    }
}
