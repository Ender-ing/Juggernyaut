/**
 * @brief
 * Manage the compiler's basic info
**/

#include "info.hpp"

#include "common/os.hpp"

namespace Base {
    namespace Info {
        // Version info
        const std::string version = MAIN_TARGET_BINARY_VERSION;

        // Installation info
        const std::string arch = Common::OS::getBuildArch();
        const std::string build = MAIN_BINARY_TYPE;

        // OS info
        const std::string osName = Common::OS::getOSName();
        const std::string osVersion = Common::OS::getOSVersion();
    }
}
