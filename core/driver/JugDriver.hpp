/**
 * @brief 
 * Front-End Driver
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_DRIVER_API

// Host/System
#include "../data/host/VFS.hpp"

// Environment data
#include "environment.hpp"

// Sessions
#include "../session/Session.hpp"

#include "Hooks.hpp"
#include "Unit.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

// TO-DO: Enforce file ownership to the closest jug.toml environment!

namespace Driver::JugDriver {
    using SetupId = Environment::EnvironmentId;
    using Setup = Environment::Environment;

    using Diagnostic = Data::Diagnostics::Diagnostic;

    struct HostConfigs {
        Data::Host::VFS *vfs = nullptr; // Virtual File System
    };

    struct DriverConfigs {
        const std::string librariesPath = ""; // Absolute path to the root libraries directory!
        Hooks hooks;
        unsigned int maxThreadsAllowed = 0; // 0 means the driver decides automatically
    };

    class JUG_DRIVER_API JugDriver {
        public:
            // Provided Configs
            HostConfigs host;
            DriverConfigs configs;
            const Data::Diagnostics::Event diagnosticEmitter = [this](const Diagnostic &d) {
                this->emitDiagnostic(d);
            };

        private:
            // Project & session graph (mutated exclusively by the thread owning the `isBusy` lease)
            std::vector<std::unique_ptr<Setup>> setups;
            std::vector<Unit> units;
            std::vector<std::vector<UnitId>> executionWaves;
            std::vector<UnitId> entryPoints;

            // Concurrency & lease coordination:
            // `stateMtx` is a strict leaf lock guarding `isBusy`, `rerunRequested`, and `pendingPaths`.
            // Never hold `stateMtx` while calling into `VFS` or acquiring `hookMtx`.
            mutable std::mutex stateMtx;
            std::atomic<bool> busyFlag{false}; // Lock-free mirror of `isBusy` for `busy()` queries
            bool isBusy = false;               // Exclusive lease over `setups` / `units` / `entryPoints`
            bool rerunRequested = false;       // Coalesces `run()` calls that arrive while `isBusy` is held
            std::vector<std::string> pendingPaths;
            std::mutex hookMtx;                // Serializes outgoing `driver` and `unitSources` hook callbacks

            // Exclusive-lease internal helpers (caller must own `isBusy` lease inside VFS Safe Mode)
            bool finishLease(bool runPass) ;
            void drainPendingPathsExclusive() ;
            void processPathsExclusive(const std::vector<std::string> &paths) ;
            void addProjectExclusive(const std::string &path) ;
            void addStrayFileExclusive(const std::string &path) ;
            bool dirConfigLookup(Data::Host::VFS *vfs, const std::string &uri,
                const std::string &sourceUri, std::string &outputUri) ;
            UnitId createUnit(const Setup *setup) ;
            void loadProject(const std::string &path) ; // Assumes the path has been validated!

            // Lifetime Management (Internal, exclusive-lease only)
            void refreshSetups() ;          // Checks jug.toml files for updates
            bool refreshDependencyGraph() ; // Updates dependency graph for parallel processing
            void runUnits() ;

            // Serialized hook dispatchers (`hookMtx`)
            void emitEnvironmentLoad(const Data::Host::File *file) ;
            void emitEnvironmentChange(const Data::Host::File *file) ;
            void emitRunFinished() ;
            void emitUnitContextSkip(const Data::Host::File *file) ;
            void emitUnitContextStart(const Data::Host::File *file) ;
            void emitUnitParserContextEnd(const Data::Host::File *file) ;
            void emitUnitContextEnd(const Data::Host::File *file) ;
            void emitUnitContextDetached(const Data::Host::File *file) ;

        public:
            // NOTE: When the driver is busy(), never change the state of the inputed variables or their children!
            explicit JugDriver(HostConfigs hostConfigs, DriverConfigs driverConfigs) ;
            virtual ~JugDriver() = default;

            // Explicitly delete copy constructor and assignment!
            JugDriver(const JugDriver&) = delete;
            JugDriver& operator=(const JugDriver&) = delete;

            // Lifetime Management (Always causes the driver to become busy!)
            // VFS is switched to "Safe Mode" when the function runs!
            // If called while busy, coalesces a follow-up pass on the active runner and returns `false`.
            bool run() ;

            // Adding projects/stray files (thread-safe; queued into `pendingPaths` when busy)
            void processPaths(std::vector<std::string> &paths) ;
            void addProject(const std::string &path) ;
            void addStrayFile(const std::string &path) ;

            // Tools
            std::string lookupProject(const std::string &path, const std::string &previous = "") ;

            // Hooks
            void emitDiagnostic(const Diagnostic &diag) ;

            // Queries
            bool busy() const noexcept;
            std::vector<Diagnostic> getDriverDiagnostics() ;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
