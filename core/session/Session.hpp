/**
 * @brief 
 * Front-End Session
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_SESSION_API

// Host & Environment
#include "../data/host/VFS.hpp"
#include "../data/driver/Environment.hpp"
#include "../data/parser/AST.hpp"

// Pipeline Stages
#include "../parser/Parser.hpp"

#include "Hooks.hpp"
#include "Configs.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Session {
    using File = Data::Host::File;
    using FileVersion = Data::Host::FileVersion;
    using SourceId = Data::Host::FileId;
    using VFS = Data::Host::VFS;
    using Environment = Data::Driver::Environment;

    using Diagnostic = Diagnostics::Diagnostic;
    using ASTNode = Data::Parser::ASTNode;
    using SymbolTable = bool; // TO-DO

    struct CompilationUnitState {
        const File *file = nullptr;
        Stage completedStage = Stage::Unprocessed;
        FileVersion contentRevision = 0; // Synced against VFS file revision

        bool hasSyntaxErrors = false;
        bool hasSemanticErrors = false;

        bool isManual = false; // Set to 'true' by using direct methods to add the file!
        bool isEntryPoint = false;
        bool isReachable = false;
        uint8_t detachedGenerations = 0;

        // (CompilationUnitState) relation tracking
        std::vector<SourceId> dependencies;
        std::vector<SourceId> dependents;

        // Data
        std::unique_ptr<ASTNode> astTable = nullptr;
    };

    class JUG_SESSION_API Session {
        private:
            static constexpr uint8_t MAX_DETACHED_GRACE = 10;

            // Basic context objects
            VFS *vfs = nullptr;
            const Environment *env = nullptr; // Rules from jug.toml

            // Read-only views into completed dependency sessions (keyed by package/alias)
            std::unordered_map<std::string, const SymbolTable*> externalPackages;

            // Session-Owned Compilation Artifacts (indexed by SourceId)
            std::unordered_map<SourceId, CompilationUnitState> units;
            std::vector<SourceId> entryPoints;

            // Execution State
            Configs configs;
            Hooks hooks;
            bool isRunning = false;
            bool hasErrors = false;

            // Internal stage & dependency helpers
            Parser::Parser createConfiguredParser() ;
            void resolveUnitImports(const File *sourceFile, CompilationUnitState &unitState, const std::vector<Parser::ImportRequest> &imports, std::vector<SourceId> &worklist) ;
            bool runParserStage() ;

        public:
            // NOTE: Session functions may operate on the assumption that 'hostVFS' and 'unitEnv' actually link to proper memory addresses!
            Session(VFS *hostVFS, const Environment *unitEnv, Configs sessionConfigs = {}, Hooks sessionHooks = {})
                : vfs(hostVFS), env(unitEnv), configs(std::move(sessionConfigs)), hooks(std::move(sessionHooks)) {}
            ~Session() = default;

            // Non-copyable, movable
            Session(const Session&) = delete;
            Session& operator=(const Session&) = delete;
            Session(Session&&) noexcept = default;
            Session& operator=(Session&&) noexcept = default;

            // (Internal) Management
            // NOTE: The Diagnostic is MOVED!
            void emitDiagnostic(Diagnostic &diag) ;

            // Source & Dependency Registration (Called by JugDriver)
            void addEntryPoint(const File *file) ;
            void refreshEntryPoints() ;

            // Pipeline
            void run();
            void rejuvenate(); // ...maybe this should be removed?

            // Queries & Cross-Session Exports
            bool succeeded() const noexcept { return !this->hasErrors; }
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
