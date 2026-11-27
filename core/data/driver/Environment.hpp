/**
 * @brief 
 * Driver Unit Configurations/Environment
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

// VFS
#include "../host/VFS.hpp"

namespace Data {
    namespace Driver {
        struct PackageConfigs {
            std::string name = "PACKAGE_NAME";
            std::string publisher = "PACKAGE_PUBLISHER";
            std::string version = "0.0.1";
            bool isDeprecated = false;

            bool operator==(const PackageConfigs&) const = default;
        };
        struct BuildFlags {
            bool ignoreWarnings = false;
            bool treatWarningsAsErrors = false;

            bool operator==(const BuildFlags&) const noexcept = default;
        };
        struct BuildConfigs {
            BuildFlags flags;
            std::vector<std::string> inputPaths;
            std::string exportPath;

            bool operator==(const BuildConfigs&) const = default;
        };
        // Dependencies
        struct LocalDependency {
            std::string name = "";
            std::string path = "";

            bool operator==(const LocalDependency&) const = default;
        };
        struct OnlineDependency {
            std::string name = "";
            std::string publisher = "";
            std::string version = "";

            bool operator==(const OnlineDependency&) const = default;
        };
        // (Internal) Debug
        struct DebugConfigs {
            bool sayMeow = false;
            bool reportCSTData = false;
            bool reportASTData = false;
            bool reportUnusedCSTNodes = false;

            bool operator==(const DebugConfigs&) const noexcept = default;
        };
        // 'Environment' is derived from the 'jug.toml' configuration files!
        using EnvironmentId = uint32_t;
        using EnvironmentVersion = uint32_t;
        using File = Data::Host::File;
        using FileVersion = Data::Host::FileVersion;
        struct Environment {
            const File* file = nullptr;     // jug.toml
            FileVersion fileVersion = 0;    // Compared against `file->version` by `JugDriver`
            EnvironmentVersion version = 0; // Incremented only when compilation-affecting configs change
            PackageConfigs package;
            BuildConfigs build;
            std::vector<LocalDependency> localDeps;
            std::vector<OnlineDependency> onlineDeps;
            // (Internal)
            DebugConfigs debug;
        };
    }
}
