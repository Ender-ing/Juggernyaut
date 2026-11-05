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
        };
        struct BuildFlags {
            bool ignoreWarnings = false;
            bool treatWarningsAsErrors = false;
        };
        struct BuildConfigs {
            BuildFlags flags;
            std::vector<std::string> inputPaths;
            std::string exportPath;
        };
        // Dependencies
        struct LocalDependency {
            std::string name = "";
            std::string path = "";
        };
        struct OnlineDependency {
            std::string name = "";
            std::string publisher = "";
            std::string version = "";
        };
        // (Internal )Debug
        struct DebugConfigs {
            bool reportAmbiguity = false;
            bool reportANTLRData = false;
        };
        // 'Environment' is derived from the 'jug.toml' configuration files!
        using EnvironmentId = uint32_t;
        using EnvironmentVersion = uint32_t;
        using File = Data::Host::File;
        struct Environment {
            const File* file= nullptr; // jug.toml
            EnvironmentVersion version = 0; // Compared against file->version!
            PackageConfigs package;
            BuildConfigs build;
            std::vector<LocalDependency> localDeps;
            std::vector<OnlineDependency> onlineDeps;
            // (Internal)
            DebugConfigs debug;
        };
    }
}
