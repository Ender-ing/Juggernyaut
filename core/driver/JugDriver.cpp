/**
 * @brief 
 * Front-End Driver
**/

#include "common/systemHeaders.hpp"
#include "JugDriver.hpp"

// 'jug.toml'-related
#include "environment.hpp"

namespace Driver::JugDriver {
    // Exclusive-lease lifecycle coordinator
    bool JugDriver::finishLease(bool runPass) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            std::lock_guard<std::mutex> lock(this->stateMtx);
            this->pendingPaths.clear();
            this->rerunRequested = false;
            this->isBusy = false;
            this->busyFlag.store(false, std::memory_order_release);
            return false;
        }

        bool safeModeActive = false;
        auto enterSafeMode = [&]() {
            if (!safeModeActive) {
                vfs->setSafeMode(true);
                safeModeActive = true;
            }
        };
        auto exitSafeMode = [&]() {
            if (safeModeActive) {
                vfs->setSafeMode(false);
                safeModeActive = false;
            }
        };

        // Exception-safety guard so an unexpected throw never strands VFS Safe Mode or `isBusy`
        struct LeaseGuard {
            JugDriver *self;
            Data::Host::VFS *vfsPtr;
            bool *safeActive;
            bool ownsLease = true;

            ~LeaseGuard() noexcept {
                if (!this->ownsLease) {
                    return;
                }
                if (*this->safeActive) {
                    this->vfsPtr->setSafeMode(false);
                    *this->safeActive = false;
                }
                std::lock_guard<std::mutex> lock(this->self->stateMtx);
                this->self->isBusy = false;
                this->self->busyFlag.store(false, std::memory_order_release);
            }
        } guard{this, vfs, &safeModeActive, true};

        bool ranAnyPass = false;

        while (true) {
            enterSafeMode();

            // Drain queued project/file registrations inside Safe Mode so `toml::parse` views stay immutable
            this->drainPendingPathsExclusive();

            if (runPass) {
                this->refreshSetups();
                if (this->refreshDependencyGraph()) {
                    this->runUnits();
                    ranAnyPass = true;
                }
            }

            // Exit VFS Safe Mode BEFORE releasing `isBusy` so staged VFS edits flush atomically
            // before any concurrent `run()` can increment `VFS::safetyCount` (preventing 1 -> 2 -> 1 skip).
            exitSafeMode();

            if (runPass) {
                this->emitRunFinished();
            }

            {
                std::lock_guard<std::mutex> lock(this->stateMtx);
                if (this->rerunRequested) {
                    this->rerunRequested = false;
                    runPass = true;
                    continue;
                }
                if (!this->pendingPaths.empty()) {
                    // Drain paths queued while `runUnits()` / `onRunFinished()` was executing
                    continue;
                }

                this->isBusy = false;
                this->busyFlag.store(false, std::memory_order_release);
                guard.ownsLease = false;
                break;
            }
        }

        return ranAnyPass;
    }
    // Internal (caller owns `isBusy` lease inside VFS Safe Mode)
    void JugDriver::drainPendingPathsExclusive() {
        while (true) {
            std::vector<std::string> batch;
            {
                std::lock_guard<std::mutex> lock(this->stateMtx);
                if (this->pendingPaths.empty()) {
                    break;
                }
                batch.swap(this->pendingPaths);
            }
            this->processPathsExclusive(batch);
        }
    }
    void JugDriver::processPathsExclusive(const std::vector<std::string> &paths) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            return;
        }

        for (const std::string &path : paths) {
            if (vfs->isDirectory(path) || vfs->filename(path) == "jug.toml") {
                this->addProjectExclusive(path);
            } else {
                this->addStrayFileExclusive(path);
            }
        }
    }
    void JugDriver::addProjectExclusive(const std::string &path) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            return;
        }

        if (!vfs->exists(path)) {
            SET_CODE(err, Error, 700001, path);
            this->emitDiagnostic(err);
        } else if (vfs->filename(path) == "jug.toml") {
            this->loadProject(path);
        } else if (vfs->isDirectory(path)) {
            std::string configUri;
            if (this->dirConfigLookup(vfs, path, path, configUri)) {
                this->loadProject(configUri);
            }
        } else {
            SET_CODE(err, Error, 700003, path);
            this->emitDiagnostic(err);
        }
    }
    void JugDriver::addStrayFileExclusive(const std::string &path) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            return;
        }

        if (!vfs->exists(path)) {
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

    bool JugDriver::dirConfigLookup(
        Data::Host::VFS *vfs,
        const std::string &uri,
        const std::string &sourceUri,
        std::string &outputUri) {
        const std::string configUri = vfs->join(uri, "jug.toml");
        const std::string parentDir = vfs->getParentDirectory(uri);

        if (vfs->exists(configUri)) {
            outputUri = configUri;
            return true;
        }
        if (parentDir.empty() || parentDir == uri) {
            SET_CODE(err, Error, 700002, sourceUri);
            this->emitDiagnostic(err);
            return false;
        }
        return this->dirConfigLookup(vfs, parentDir, sourceUri, outputUri);
    }
    UnitId JugDriver::createUnit(const Setup *setup) {
        const UnitId id = static_cast<UnitId>(this->units.size());
        this->units.emplace_back(this->host.vfs, setup, UnitConfigs{}, UnitHooks{
            .onContextSkip = [this](const Data::Host::File *file) {
                this->emitUnitContextSkip(file);
            },
            .onContextStart = [this](const Data::Host::File *file) {
                this->emitUnitContextStart(file);
            },
            .onParserContextEnd = [this](const Data::Host::File *file) {
                this->emitUnitParserContextEnd(file);
            },
            .onContextEnd = [this](const Data::Host::File *file) {
                this->emitUnitContextEnd(file);
            },
            .onContextDetached = [this](const Data::Host::File *file) {
                this->emitUnitContextDetached(file);
            },
            .onDiagnostic = [this](const Diagnostic &diag) {
                this->emitDiagnostic(diag);
            }
        });
        return id;
    }
    void JugDriver::loadProject(const std::string &path) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            return;
        }

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
        this->setups.push_back(
            std::make_unique<Setup>(Environment::fetchEnvironment(vfs, this->diagnosticEmitter, path))
        );
        const Setup *setup = this->setups.back().get();

        // Fire relevant events
        this->emitEnvironmentLoad(file);

        // Create a unit!
        this->entryPoints.push_back(this->createUnit(setup));

        // Declare the project to the user!
        SET_CODE(diag, Info, 700018, path);
        this->emitDiagnostic(diag);
    }

    // Lifetime Management (Internal)
    void JugDriver::refreshSetups() {
        for (std::unique_ptr<Setup> &setup : this->setups) {
            if (setup != nullptr && setup->file != nullptr && setup->file->version > setup->version) {
                Environment::refreshEnvironment(this->host.vfs, setup.get(), this->diagnosticEmitter);
                this->emitEnvironmentChange(setup->file);
            }
        }
    }
    bool JugDriver::refreshDependencyGraph() {
        // PLANNED: Topological Wave (Level) Scheduling (Kahn's algorithm)
        // NOTE: Project dependencies haven't been implemented yet!
        // As such, this is a dummy call for the time being that adds one
        // execution layer!

        this->executionWaves.clear();

        if (!this->entryPoints.empty()) {
            this->executionWaves.push_back(this->entryPoints);
        }

        return true;
    }
    void JugDriver::runUnits() {
        // Execute each topological wave in order
        const unsigned int systemThreadMax = std::max(1u, std::thread::hardware_concurrency());
        const unsigned int maxThreads = this->configs.maxThreadsAllowed == 0
            ? systemThreadMax
            : std::min(this->configs.maxThreadsAllowed, systemThreadMax);

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

    // Serialized Hook Dispatchers
    void JugDriver::emitDiagnostic(const Diagnostic &diag) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.driver.onDiagnostic != nullptr) {
            this->configs.hooks.driver.onDiagnostic(diag);
        }
    }
    void JugDriver::emitEnvironmentLoad(const Data::Host::File *file) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.driver.onEnvironmentLoad != nullptr) {
            this->configs.hooks.driver.onEnvironmentLoad(file);
        }
    }
    void JugDriver::emitEnvironmentChange(const Data::Host::File *file) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.driver.onEnvironmentChange != nullptr) {
            this->configs.hooks.driver.onEnvironmentChange(file);
        }
    }
    void JugDriver::emitRunFinished() {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.driver.onRunFinished != nullptr) {
            this->configs.hooks.driver.onRunFinished();
        }
    }
    void JugDriver::emitUnitContextSkip(const Data::Host::File *file) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.unitSources.onContextSkip != nullptr) {
            this->configs.hooks.unitSources.onContextSkip(file);
        }
    }
    void JugDriver::emitUnitContextStart(const Data::Host::File *file) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.unitSources.onContextStart != nullptr) {
            this->configs.hooks.unitSources.onContextStart(file);
        }
    }
    void JugDriver::emitUnitParserContextEnd(const Data::Host::File *file) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.unitSources.onParserContextEnd != nullptr) {
            this->configs.hooks.unitSources.onParserContextEnd(file);
        }
    }
    void JugDriver::emitUnitContextEnd(const Data::Host::File *file) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.unitSources.onContextEnd != nullptr) {
            this->configs.hooks.unitSources.onContextEnd(file);
        }
    }
    void JugDriver::emitUnitContextDetached(const Data::Host::File *file) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.unitSources.onContextDetached != nullptr) {
            this->configs.hooks.unitSources.onContextDetached(file);
        }
    }

    JugDriver::JugDriver(HostConfigs hostConfigs, DriverConfigs driverConfigs)
        : host(hostConfigs), configs(std::move(driverConfigs)) {}

    // Lifetime Management (Always causes the driver to become busy!)
    bool JugDriver::run() {
        {
            std::lock_guard<std::mutex> lock(this->stateMtx);
            if (this->isBusy) {
                this->rerunRequested = true;
                return false;
            }
            this->isBusy = true;
            this->busyFlag.store(true, std::memory_order_release);
            this->rerunRequested = false;
        }
        return this->finishLease(true);
    }

    // Adding projects/stray files
    void JugDriver::processPaths(std::vector<std::string> &paths) {
        if (paths.empty()) {
            return;
        }

        bool acquiredLease = false;
        {
            std::lock_guard<std::mutex> lock(this->stateMtx);
            this->pendingPaths.insert(this->pendingPaths.end(), paths.begin(), paths.end());
            if (!this->isBusy) {
                this->isBusy = true;
                this->busyFlag.store(true, std::memory_order_release);
                acquiredLease = true;
            }
        }
        if (acquiredLease) {
            (void)this->finishLease(false);
        }
    }
    void JugDriver::addProject(const std::string &path) {
        bool acquiredLease = false;
        {
            std::lock_guard<std::mutex> lock(this->stateMtx);
            this->pendingPaths.push_back(path);
            if (!this->isBusy) {
                this->isBusy = true;
                this->busyFlag.store(true, std::memory_order_release);
                acquiredLease = true;
            }
        }
        if (acquiredLease) {
            (void)this->finishLease(false);
        }
    }
    void JugDriver::addStrayFile(const std::string &path) {
        bool acquiredLease = false;
        {
            std::lock_guard<std::mutex> lock(this->stateMtx);
            this->pendingPaths.push_back(path);
            if (!this->isBusy) {
                this->isBusy = true;
                this->busyFlag.store(true, std::memory_order_release);
                acquiredLease = true;
            }
        }
        if (acquiredLease) {
            (void)this->finishLease(false);
        }
    }

    // Tools
    std::string JugDriver::lookupProject(const std::string &path, const std::string &previous) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr || path.empty() || path == previous) {
            return ""; // Missing VFS or reached filesystem root
        }

        if (vfs->filename(path) == "jug.toml") {
            if (vfs->exists(path)) {
                return path;
            }
            const std::string dir = vfs->getParentDirectory(path);
            return this->lookupProject(vfs->getParentDirectory(dir), dir);
        }
        if (vfs->isDirectory(path)) {
            const std::string newPath = vfs->join(path, "jug.toml");
            if (vfs->exists(newPath)) {
                return newPath;
            }
            return this->lookupProject(vfs->getParentDirectory(path), path);
        }
        return this->lookupProject(vfs->getParentDirectory(path), path);
    }

    // Queries
    bool JugDriver::busy() const noexcept {
        return this->busyFlag.load(std::memory_order_acquire);
    }
    std::vector<Diagnostic> JugDriver::getDriverDiagnostics() {
        return {};
    }
}
