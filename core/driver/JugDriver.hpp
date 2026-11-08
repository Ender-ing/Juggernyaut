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

namespace Driver {
    namespace JugDriver {
        using SetupId = Environment::EnvironmentId;
        using Setup = Environment::Environment;

        using Diagnostic = Diagnostics::Diagnostic;

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
                const Diagnostics::Event diagnosticEmitter = [this](const Diagnostic& d) { this->emitDiagnostic(d); };
            private:
                // Internal variables
                std::vector<std::unique_ptr<Setup>> setups;
                std::vector<Unit> units;
                std::vector<std::vector<UnitId>> executionWaves;
                std::vector<UnitId> entryPoints;

                bool isBusy = false;
                std::vector<std::string> pendingPaths;
                std::mutex hookMtx;

                // Internal
                void processPendingPaths() ;
                bool dirConfigLookup(DriverHooks &driverHooks, Data::Host::VFS *vfs, const std::string &uri, const std::string &sourceUri, std::string &outputUri) ;
                UnitId createUnit(const Setup *setup) ;
                void loadProject(const std::string &path) ; // Assumes the path has been validated!

                // Lifetime Management (Internal)
                void refreshSetups() ; // Checks jug.toml files for updates
                bool refreshDependencyGraph() ; // Updates dependency graph for parallel processing
                void runUnits() ;
            public:
                // NOTE: When the driver is busy(), never change the state of the inputed variables or their children!
                JugDriver(HostConfigs hostConfigs, DriverConfigs driverConfigs) : host(hostConfigs), configs(std::move(driverConfigs)) {};
                virtual ~JugDriver() = default;

                // Explicitly delete copy constructor and assignment!
                JugDriver(const JugDriver&) = delete;
                JugDriver& operator=(const JugDriver&) = delete;
                // Explicitly allow moving
                //JugDriver(JugDriver&&) noexcept = default;
                //JugDriver& operator=(JugDriver&&) noexcept = default;

                // Lifetime Management (Always cause the driver to become busy!)
                // VFS is switched to "Safe Mode" when the function runs!
                bool run() ; // Returns true if the driver got triggered!

                // Adding projects/stary files
                // NOTE: These actions will be queued when the driver is busy!
                void processPaths(std::vector<std::string> &paths) ;
                void addProject(const std::string &path) ;
                void addStrayFile(const std::string &path) ;

                // Tools
                std::string lookupProject(const std::string &path, const std::string &previous = "") ; // Look up a parent project path

                // Hooks
                void emitDiagnostic(const Diagnostic &diag) ;

                // Queries
                bool busy() const noexcept { return this->isBusy; }
                std::vector<Diagnostic> getDriverDiagnostics() ;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
