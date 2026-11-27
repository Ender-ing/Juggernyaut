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

// Parser
#include "../data/parser/SharedCSTCache.hpp"
#include "../parser/Parser.hpp"

#include "Hooks.hpp"
#include "Configs.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Session {
    // Host
    using File = Data::Host::File;
    using FileVersion = Data::Host::FileVersion;
    using SourceId = Data::Host::FileId;
    using VFS = Data::Host::VFS;

    // Driver
    using Environment = Data::Driver::Environment;
    using EnvironmentVersion = Data::Driver::EnvironmentVersion;

    // Diagnostics
    using Diagnostic = Data::Diagnostics::Diagnostic;

    // Parser
    using SharedCSTCache = Data::Parser::SharedCSTCache;
    using AST = Data::Parser::AST;

    // Semantic
    using SymbolTable = bool; // TO-DO

    struct ExternalPackage {
        std::string alias;
        const SymbolTable *symbols = nullptr;
    };

    struct CompilationUnitState {
        const File *file = nullptr;

        // Data
        std::unique_ptr<AST> astTable = nullptr;

        // (CompilationUnitState) relation tracking
        std::vector<SourceId> dependencies;
        std::vector<SourceId> dependents;

        FileVersion contentRevision = 0; // Synced against VFS file revision
        Stage completedStage = Stage::Unprocessed;
        uint8_t detachedGenerations = 0;

        bool hasSyntaxErrors : 1 = false;
        bool hasSemanticErrors : 1 = false;
        bool isManual : 1 = false; // Set to 'true' by using direct methods to add the file!
        bool isEntryPoint : 1 = false;
        bool isReachable : 1 = false;
        bool isVisited : 1 = false;
    };

    class JUG_SESSION_API Session {
        private:
            static constexpr uint8_t MAX_DETACHED_GRACE = 10;

            // Basic context objects
            VFS *vfs = nullptr;
            const Environment *env = nullptr; // Rules from jug.toml
            EnvironmentVersion envRevision = 0; // Synced against env->version

            // Cache
            std::unique_ptr<Data::Parser::SharedCSTCache> cstCache = nullptr;

            // Read-only views into completed dependency sessions (keyed by package/alias)
            std::vector<ExternalPackage> externalPackages;

            // Session-Owned Compilation Artifacts
            std::vector<CompilationUnitState> units;
            std::vector<SourceId> entryPoints;

            // Execution State
            Hooks hooks;
            Configs configs;
            bool isRunning : 1 = false;
            bool hasErrors : 1 = false;

            // Internal stage & dependency helpers
            void refreshEntryPoints() ;
            void invalidateCompilationUnits() ;

            // Parser Stage Helpers
            Parser::Parser createConfiguredParser() ;
            void resolveUnitImports(const File *sourceFile, CompilationUnitState &unitState, const std::vector<Parser::ImportRequest> &imports, std::vector<SourceId> &worklist) ;
            bool runParserStage() ;

        public:
            // NOTE: Session functions may operate on the assumption that 'hostVFS' and 'unitEnv' actually link to proper memory addresses!
            Session(VFS *hostVFS, const Environment *unitEnv, Configs sessionConfigs = {}, Hooks sessionHooks = {})
                : vfs(hostVFS), env(unitEnv), hooks(std::move(sessionHooks)), configs(std::move(sessionConfigs)) {

                    // Allow caching
                    if (this->configs.allowCSTCache) {
                        this->cstCache = std::make_unique<SharedCSTCache>();
                    }
                }
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

            // Pipeline
            void run() ;
            void rejuvenate() ; // ...maybe this should be removed?

            // Queries & Cross-Session Exports
            bool succeeded() const noexcept { return !this->hasErrors; }
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
