/**
 * @brief 
 * Front-End Driver
**/

#include "common/systemHeaders.hpp"
#include "JugDriver.hpp"

// 'jug.toml'-related
#include "environment.hpp"

namespace Driver {
    namespace JugDriver {
        // Internal
        void JugDriver::processPendingPaths() {
            // Drain any paths that were added whilst a previous run was busy
            if (!this->pendingPaths.empty()) {
                std::vector<std::string> queued = std::move(this->pendingPaths);
                this->pendingPaths.clear();
                this->processPaths(queued);
            }
        }
        bool JugDriver::dirConfigLookup(DriverHooks &driverHooks, Data::Host::VFS *vfs, const std::string &uri, const std::string &sourceUri, std::string &outputUri) {
            const std::string configUri = vfs->join(uri, "jug.toml");
            const std::string parentDir = vfs->getParentDirectory(uri);

            if (vfs->exists(configUri)) {
                outputUri = std::move(configUri);
                return true;
            } else if (parentDir == uri) {
                SET_CODE(err, Error, 700002, sourceUri);
                this->emitDiagnostic(err);
                return false;
            } else {
                return dirConfigLookup(driverHooks, vfs, parentDir, sourceUri, outputUri);
            }
        }
        UnitId JugDriver::createUnit(const Setup *setup) {
            UnitId id = static_cast<UnitId>(this->units.size());
            this->units.emplace_back(this->host.vfs, setup, UnitConfigs{}, UnitHooks{
                .onContextSkip = this->configs.hooks.unitSources.onContextSkip,
                .onContextStart = this->configs.hooks.unitSources.onContextStart,
                .onLexerContextEnd = this->configs.hooks.unitSources.onLexerContextEnd,
                .onParserContextEnd = this->configs.hooks.unitSources.onParserContextEnd,
                .onContextEnd = this->configs.hooks.unitSources.onContextEnd,
                .onContextDetached = this->configs.hooks.unitSources.onContextDetached,
                .onDiagnostic = [this](const Diagnostic &diag){
                    this->emitDiagnostic(diag);
                }
            });
            return id;
        }
        void JugDriver::loadProject(const std::string &path) {
            // Reference needed objects
            DriverHooks &driverHooks = this->configs.hooks.driver;
            Data::Host::VFS *vfs = (this->host).vfs;

            // Get the environment's source file
            const Data::Host::File *file = vfs->readFile(path);
            if (file == nullptr) {
                return;
            }

            // Skip if environment is already loaded
            for (const auto &existing : this->setups) {
                if (existing != nullptr && existing->file == file) {
                    return;
                }
            }

            // Load a new environment
            this->setups.push_back(std::make_unique<Setup>(Environment::fetchEnvironment(vfs, this->diagnosticEmitter, path)));
            Setup *setup = this->setups.back().get();

            // Fire relevant events
            if (driverHooks.onEnvironmentLoad != nullptr) {
                driverHooks.onEnvironmentLoad(file);
            }

            // Create a unit!
            this->entryPoints.push_back(createUnit(setup));

            // Declare the project to the user!
            SET_CODE(diag, Info, 700018, path);
            this->emitDiagnostic(diag);
        }

        // Lifetime Management (Internal)
        void JugDriver::refreshSetups() {
            for (std::unique_ptr<Setup> &setup : this->setups) {
                if (setup->file != nullptr && setup->file->version > setup->version) {
                    Environment::refreshEnvironment(this->host.vfs, setup.get(), this->diagnosticEmitter);

                    // Fire relevant events
                    if (this->configs.hooks.driver.onEnvironmentChange != nullptr) {
                        this->configs.hooks.driver.onEnvironmentChange(setup->file);
                    }
                }
            }
        }
        bool JugDriver::refreshDependencyGraph() {
            // PLANNED: Topological Wave (Level) Scheduling (Kahn's algorithm)
            // NOTE: Project dependencies haven't been implemented yet!
            // As such, this is a dummy call for the time being that adds one
            // execution layer!

            // Reset waves
            executionWaves.clear();

            // Dummy code
            if (!this->entryPoints.empty()) {
                this->executionWaves.push_back(this->entryPoints);
            }

            return true;
        }
        void JugDriver::runUnits() {
            // Execute each topological wave in order
            const unsigned int systemThreadMax = std::max(1u, std::thread::hardware_concurrency());
            const unsigned int maxThreads = this->configs.maxThreadsAllowed == 0 ?
                systemThreadMax :
                std::min(this->configs.maxThreadsAllowed, systemThreadMax);

            for (const std::vector<UnitId> &wave : this->executionWaves) {
                if (wave.empty()) {
                    continue;
                }

                // Handle single jobs without spawning a thread
                if (wave.size() == 1) {
                    this->units[wave[0]].run();
                    continue;
                }

                // Run independent units in parallel
                std::atomic<size_t> nextIdx{0};
                const size_t waveSize = wave.size();
                const size_t workerCount = std::min<size_t>(waveSize, maxThreads);
                std::vector<std::jthread> workers;
                workers.reserve(workerCount);
                for (size_t w = 0; w < workerCount; ++w) {
                    workers.emplace_back([this, &wave, waveSize, &nextIdx]() {
                        while (true) {
                            const size_t idx = nextIdx.fetch_add(1, std::memory_order_relaxed);
                            if (idx >= waveSize) {
                                break;
                            }
                            this->units[wave[idx]].run();
                        }
                    });
                }

                // NOTE: 'std::jthread' destructors automatically join all workers!
            }
        }

        // Lifetime Management (Always cause the driver to become busy!)
        // VFS is switched to "Safe Mode" when the function runs!
        bool JugDriver::run() {
            if (this->isBusy) {
                return false;
            }

            // Check for pending path jobs
            this->processPendingPaths();

            // Enter VFS Safe Mode
            this->isBusy = true;
            this->host.vfs->setSafeMode(true);

            // Handle exit logic
            struct RunCleanup {
                JugDriver *self;
                ~RunCleanup() {
                    // Exit VFS Safe Mode
                    self->isBusy = false;
                    self->host.vfs->setSafeMode(false);

                    // Trigger relevent events
                    if (self->configs.hooks.driver.onRunFinished != nullptr) {
                        self->configs.hooks.driver.onRunFinished();
                    }
                }
            } cleanup{this};

            // Make sure jug.toml configs are up-to-date
            this->refreshSetups();

            // (Re)construct the dependency graph
            if(!(this->refreshDependencyGraph())) {
                return false;
            }

            // Initiate parallel independent session runs!
            this->runUnits();

            return true;
        }

        // Adding projects/stary files
        void JugDriver::processPaths(std::vector<std::string> &paths) {
            // Reference needed objects
            Data::Host::VFS *vfs = (this->host).vfs;

            // Prevent changes during a run!
            if (this->isBusy) {
                this->pendingPaths.insert(this->pendingPaths.end(), paths.begin(), paths.end());
                return;
            }
            this->processPendingPaths();

            for (const std::string &path : paths) {
                if (vfs->isDirectory(path) || vfs->filename(path) == "jug.toml") {
                    this->addProject(path);
                } else {
                    this->addStrayFile(path);
                }
            }
        }
        void JugDriver::addProject(const std::string &path) {
            // Reference needed objects
            DriverHooks &driverHooks = this->configs.hooks.driver;
            Data::Host::VFS *vfs = (this->host).vfs;

            // Prevent changes during a run!
            if (this->isBusy) {
                this->pendingPaths.push_back(path);
                return;
            }
            this->processPendingPaths();

            if (!(vfs->exists(path))) {
                // Couldn't find the specified uri
                SET_CODE(err, Error, 700001, path);
                this->emitDiagnostic(err);
            } else if (vfs->filename(path) == "jug.toml") {
                // Load the project
                this->loadProject(path);
            } else if (vfs->isDirectory(path)) {
                std::string configUri;
                if(dirConfigLookup(driverHooks, vfs, path, path, configUri)) {
                    // Load the project
                    this->loadProject(configUri);
                } else {
                    SET_CODE(err, Error, 700002, path);
                    this->emitDiagnostic(err);
                }
            } else {
                SET_CODE(err, Error, 700003, path);
                this->emitDiagnostic(err);
            }
        }
        void JugDriver::addStrayFile(const std::string &path) {
            // Reference needed objects
            // DriverHooks &driverHooks = this->configs.hooks.driver;
            Data::Host::VFS *vfs = (this->host).vfs;

            // Prevent changes during a run!
            if (this->isBusy) {
                this->pendingPaths.push_back(path);
                return;
            }
            this->processPendingPaths();

            if (!(vfs->exists(path))) {
                // Couldn't find the specified uri
                SET_CODE(err, Error, 700001, path);
                this->emitDiagnostic(err);
            } else if (vfs->endsWith(path, ".jug")) {
                // ADD THE FILE!
                // MUST CHECK FOR CONFIGS FIRST!
            } else {
                SET_CODE(err, Error, 700004, path);
                this->emitDiagnostic(err);
            }
        }

        // Tools
        std::string JugDriver::lookupProject(const std::string &path, const std::string &previous) {
            if (path.empty() || path == previous) {
                return ""; // Reached filesystem root
            }
            Data::Host::VFS *vfs = this->host.vfs;

            if (vfs->filename(path) == "jug.toml") {
                if (vfs->exists(path)) {
                    return path;
                }
                const std::string dir = vfs->getParentDirectory(path);

                return this->lookupProject(vfs->getParentDirectory(dir), dir);
            } else if (vfs->isDirectory(path)) {
                const std::string newPath = vfs->join(path, "jug.toml");
                if (vfs->exists(newPath)) {
                    return newPath;
                }

                return this->lookupProject(vfs->getParentDirectory(path), path);
            } else {
                return this->lookupProject(vfs->getParentDirectory(path), path);
            }
        }

        // Hooks
        void JugDriver::emitDiagnostic(const Diagnostic &diag) {
            if (this->configs.hooks.driver.onDiagnostic != nullptr) {
                std::lock_guard<std::mutex> lock(this->hookMtx); // Serialise hook calls!
                this->configs.hooks.driver.onDiagnostic(diag);
            }
        }
    }
}
