/**
 * @brief 
 * Front-End Driver
**/

#include "common/systemHeaders.hpp"
#include "JugDriver.hpp"

// 'jug.toml'-related
#include "environment.hpp"

namespace Driver::JugDriver {
    namespace Helper {
        struct SafeModeScope {
            Data::Host::VFS *vfs = nullptr;
            bool active = false;

            SafeModeScope(Data::Host::VFS *targetVfs, bool enter)
                : vfs(targetVfs), active(enter && targetVfs != nullptr) {
                if (this->active) {
                    this->vfs->setSafeMode(true);
                }
            }
            ~SafeModeScope() noexcept {
                if (this->active) {
                    this->vfs->setSafeMode(false);
                }
            }

            SafeModeScope(const SafeModeScope&) = delete;
            SafeModeScope& operator=(const SafeModeScope&) = delete;
        };

        struct BusyScope {
            std::atomic<uint32_t> *counter;

            explicit BusyScope(std::atomic<uint32_t> *target) noexcept : counter(target) {
                this->counter->fetch_add(1u, std::memory_order_release);
            }
            ~BusyScope() noexcept {
                this->counter->fetch_sub(1u, std::memory_order_release);
            }

            BusyScope(const BusyScope&) = delete;
            BusyScope& operator=(const BusyScope&) = delete;
        };
    }

    // Internal (`runMtx` held by caller)
    void JugDriver::drainPendingPaths(bool inSafeMode) {
        while (true) {
            std::vector<std::string> batch;
            {
                std::lock_guard<std::mutex> lock(this->pathsMtx);
                if (this->pendingPaths.empty()) {
                    break;
                }
                batch.swap(this->pendingPaths);
            }
            this->processPathsExclusive(batch, inSafeMode);
        }
    }
    void JugDriver::processPathsExclusive(const std::vector<std::string> &paths, bool inSafeMode) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            return;
        }

        for (const std::string &path : paths) {
            if (vfs->isDirectory(path) || vfs->filename(path) == "jug.toml") {
                this->addProjectExclusive(path, inSafeMode);
            } else {
                this->addStrayFileExclusive(path);
            }
        }
    }
    void JugDriver::addProjectExclusive(const std::string &path, bool inSafeMode) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            return;
        }

        if (!vfs->exists(path)) {
            SET_CODE(err, Error, 700001, path);
            this->emitDiagnostic(err);
        } else if (vfs->filename(path) == "jug.toml") {
            this->loadProject(path, inSafeMode);
        } else if (vfs->isDirectory(path)) {
            std::string configUri;
            if (this->dirConfigLookup(vfs, path, path, configUri)) {
                this->loadProject(configUri, inSafeMode);
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
                if (this->configs.hooks.unitSources.onContextSkip != nullptr) {
                    this->configs.hooks.unitSources.onContextSkip(file);
                }
            },
            .onContextStart = [this](const Data::Host::File *file) {
                if (this->configs.hooks.unitSources.onContextStart != nullptr) {
                    this->configs.hooks.unitSources.onContextStart(file);
                }
            },
            .onParserContextEnd = [this](const Data::Host::File *file) {
                if (this->configs.hooks.unitSources.onParserContextEnd != nullptr) {
                    this->configs.hooks.unitSources.onParserContextEnd(file);
                }
            },
            .onContextEnd = [this](const Data::Host::File *file) {
                if (this->configs.hooks.unitSources.onContextEnd != nullptr) {
                    this->configs.hooks.unitSources.onContextEnd(file);
                }
            },
            .onContextDetached = [this](const Data::Host::File *file) {
                if (this->configs.hooks.unitSources.onContextDetached != nullptr) {
                    this->configs.hooks.unitSources.onContextDetached(file);
                }
            },
            .onDiagnostic = [this](const Diagnostic &diag) {
                this->emitDiagnostic(diag);
            }
        });
        return id;
    }
    void JugDriver::loadProject(const std::string &path, bool inSafeMode) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            return;
        }

        const Data::Host::File *file = vfs->readFile(path);
        if (file == nullptr) {
            return;
        }

        // Skip if environment is already loaded (no VFS Safe Mode toggle needed)
        for (const auto &existing : this->setups) {
            if (existing != nullptr && existing->file == file) {
                return;
            }
        }

        // Guard `toml::parse(file->content.view())` if called outside `run()`
        {
            Helper::SafeModeScope safeGuard(vfs, !inSafeMode);
            this->setups.push_back(
                std::make_unique<Setup>(Environment::fetchEnvironment(vfs, this->diagnosticEmitter, path))
            );
        }
        const Setup *setup = this->setups.back().get();

        if (this->configs.hooks.driver.onEnvironmentLoad != nullptr) {
            this->configs.hooks.driver.onEnvironmentLoad(file);
        }

        this->entryPoints.push_back(this->createUnit(setup));

        SET_CODE(diag, Info, 700018, path);
        this->emitDiagnostic(diag);
    }

    // Lifetime Management (Internal)
    void JugDriver::refreshSetups() {
        for (std::unique_ptr<Setup> &setup : this->setups) {
            if (setup != nullptr && setup->file != nullptr && setup->file->version > setup->fileVersion) {
                Environment::refreshEnvironment(this->host.vfs, setup.get(), this->diagnosticEmitter);

                if (this->configs.hooks.driver.onEnvironmentChange != nullptr) {
                    this->configs.hooks.driver.onEnvironmentChange(setup->file);
                }
            }
        }
    }
    bool JugDriver::refreshDependencyGraph() {
        // PLANNED: Topological Wave (Level) Scheduling (Kahn's algorithm)
        this->executionWaves.clear();

        if (!this->entryPoints.empty()) {
            this->executionWaves.push_back(this->entryPoints);
        }

        return true;
    }
    void JugDriver::runUnits() {
        // Reserve 1 hardware thread for LSP/UI responsiveness when `maxThreadsAllowed == 0`
        const unsigned int hardwareThreads = std::max(1u, std::thread::hardware_concurrency());
        const unsigned int autoBudget = hardwareThreads > 1u ? (hardwareThreads - 1u) : 1u;
        const unsigned int maxThreads = this->configs.maxThreadsAllowed == 0u
            ? autoBudget
            : std::min(this->configs.maxThreadsAllowed, hardwareThreads);

        for (const std::vector<UnitId> &wave : this->executionWaves) {
            if (wave.empty()) {
                continue;
            }

            if (wave.size() == 1u) {
                this->units[wave[0]].run();
                continue;
            }

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
        }
    }

    JugDriver::JugDriver(HostConfigs hostConfigs, DriverConfigs driverConfigs)
        : host(hostConfigs), configs(std::move(driverConfigs)) {}

    bool JugDriver::run() {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr) {
            return false;
        }
        if (this->runningPass.exchange(true, std::memory_order_acq_rel)) {
            return false;
        }
        struct PassGuard {
            std::atomic<bool> *flag;
            ~PassGuard() noexcept { this->flag->store(false, std::memory_order_release); }
        } passGuard{&this->runningPass};

        std::lock_guard<std::mutex> lock(this->runMtx);
        Helper::BusyScope busyScope(&this->activeOwners);

        {
            // Exit VFS Safe Mode BEFORE releasing `runMtx` and BEFORE `onRunFinished`
            Helper::SafeModeScope safeScope(vfs, true);

            this->drainPendingPaths(true);
            this->refreshSetups();
            if (!this->refreshDependencyGraph()) {
                return false;
            }
            this->runUnits();
        }

        if (this->configs.hooks.driver.onRunFinished != nullptr) {
            this->configs.hooks.driver.onRunFinished();
        }

        this->drainPendingPaths(false);
        return true;
    }

    // Adding projects/stray files
    void JugDriver::processPaths(std::vector<std::string> &paths) {
        if (paths.empty()) {
            return;
        }
        std::unique_lock<std::mutex> lock(this->runMtx, std::try_to_lock);
        if (!lock.owns_lock()) {
            std::lock_guard<std::mutex> qlock(this->pathsMtx);
            this->pendingPaths.insert(this->pendingPaths.end(), paths.begin(), paths.end());
            return;
        }
        Helper::BusyScope busyScope(&this->activeOwners);
        this->drainPendingPaths(false);
        this->processPathsExclusive(paths, false);
    }
    void JugDriver::addProject(const std::string &path) {
        std::unique_lock<std::mutex> lock(this->runMtx, std::try_to_lock);
        if (!lock.owns_lock()) {
            std::lock_guard<std::mutex> qlock(this->pathsMtx);
            this->pendingPaths.push_back(path);
            return;
        }
        Helper::BusyScope busyScope(&this->activeOwners);
        this->drainPendingPaths(false);
        this->addProjectExclusive(path, false);
    }
    void JugDriver::addStrayFile(const std::string &path) {
        std::unique_lock<std::mutex> lock(this->runMtx, std::try_to_lock);
        if (!lock.owns_lock()) {
            std::lock_guard<std::mutex> qlock(this->pathsMtx);
            this->pendingPaths.push_back(path);
            return;
        }
        Helper::BusyScope busyScope(&this->activeOwners);
        this->drainPendingPaths(false);
        this->addStrayFileExclusive(path);
    }

    // Tools
    std::string JugDriver::lookupProject(const std::string &path, const std::string &previous) {
        Data::Host::VFS *vfs = this->host.vfs;
        if (vfs == nullptr || path.empty() || path == previous) {
            return "";
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

    // Hooks
    void JugDriver::emitDiagnostic(const Diagnostic &diag) {
        std::lock_guard<std::mutex> lock(this->hookMtx);
        if (this->configs.hooks.driver.onDiagnostic != nullptr) {
            this->configs.hooks.driver.onDiagnostic(diag);
        }
    }

    // Queries
    bool JugDriver::busy() const noexcept {
        return this->activeOwners.load(std::memory_order_acquire) > 0u;
    }
    std::vector<Diagnostic> JugDriver::getDriverDiagnostics() {
        return {};
    }
}
