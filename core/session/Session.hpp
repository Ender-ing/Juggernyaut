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

// Pipeline Stages
//#include "../parser/Hooks.hpp"
//#include "../parser/Configs.hpp"

#include "Hooks.hpp"
#include "Configs.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Session {
    using File = Data::Host::File;
    using FileVersion = Data::Host::FileVersion;
    using SourceId = Data::Host::FileId; // Matches the original file's ID
    using VFS = Data::Host::VFS;
    using Environment = Data::Driver::Environment;

    using Diagnostic = Diagnostics::Diagnostic;

    // TO-DO:
    using ASTNode = bool;
    using SymbolTable = bool;

    // Tracks breaking config changes that force full stage invalidation
    /*struct RevisionStates {
        uint32_t parser      = 0;
        uint32_t transformer = 0;
        uint32_t semantic    = 0;
    };*/

    // Lightweight per-file state within THIS session (replaces heavy Source class)
    struct CompilationUnitState {
        const File* file = nullptr;
        Stage completedStage = Stage::Unprocessed;
        FileVersion contentRevision = 0; // Synced against VFS file revision
        bool hasErrors = false;

        bool isEntryPoint = false;
        bool isReachable = false;

        // (CompilationUnitState) usage tracking
        std::vector<SourceId> dependencies;
        std::vector<SourceId> dependents;
    };

    class JUG_SESSION_API Session {
        private:
            // Basic context objects
            VFS *vfs = nullptr;
            const Environment *env = nullptr; // Rules from jug.toml

            // Read-only views into completed dependency sessions (keyed by package/alias)
            std::unordered_map<std::string, const SymbolTable*> externalPackages;

            // Session-Owned Compilation Artifacts
            // Indexed by SourceId
            std::unordered_map<SourceId, CompilationUnitState> units;
            // std::unordered_map<SourceId, std::unique_ptr<ASTNode>> astTable;
            // std::unordered_map<SourceId, std::unique_ptr<SymbolTable>> symbolTables;
            std::vector<SourceId> entryPoints;

            // --- 3. Execution State ---
            Configs configs;
            Hooks hooks;
            // RevisionStates revisions;
            bool isRunning = false;
            bool hasErrors = false;

            // Internal stage runners
            // bool runParserStage();
            // bool runTransformerStage();
            // bool runSemanticStage();
            // bool runIaRCStage();

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

            // (Inner) Management
            // NOTE: The Diagnostic is MOVED!
            void emitDiagnostic(const Diagnostic &diag) ;

            // Source & Dependency Registration (Called by JugDriver)
            void addEntryPoint(const File* file) ;
            void refreshEntryPoints() ;
            // void invalidateSource(SourceId id) ;
            // void linkExternalPackage(const std::string &alias, const SymbolTable *exportedSymbols);

            // Pipeline
            // void refreshDependencies() ; // Refresh all compilation unit  dependencies
            void run() ;

            // Flushe caches (ANTLR DFA caches, dead units, ...)
            // void rejuvenate();

            // Queries & Cross-Session Exports
            bool succeeded() const noexcept { return !hasErrors; }
            // const ASTNode* getAST(SourceId id) const;
            // const SymbolTable* getExportedSymbols(SourceId id) const;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
