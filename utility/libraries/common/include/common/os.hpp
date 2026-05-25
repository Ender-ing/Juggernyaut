/**
 * @brief
 * Manage os info
**/

#pragma once

#include "headers.hpp"
//#include "../../dynamic.hpp" // JUG_COMMON_API // NOT ALLOWED!

#if _WIN32
    #include <windows.h>
    #include <lm.h>
    #pragma comment(lib, "netapi32.lib")

    std::string internal_getWindowsName() {
        LPWKSTA_INFO_102 wki;
        // Fetches the true major and minor OS version numbers
        if (NetWkstaGetInfo(NULL, 102, (LPBYTE*)&wki) == NERR_Success) {
            std::string tmp = (wki->wki102_platform_id == PLATFORM_ID_NT) ? "Windows NT" : "Windows";

            NetApiBufferFree(wki);

            return tmp;
        }

        return "?Windows?";
    }
    std::string internal_getWindowsVersion() {
        LPWKSTA_INFO_102 wki;
        // Fetches the true major and minor OS version numbers
        if (NetWkstaGetInfo(NULL, 102, (LPBYTE*)&wki) == NERR_Success) {
            std::string tmp = std::to_string(wki->wki102_ver_major);
            tmp += ".";
            tmp += std::to_string(wki->wki102_ver_minor);

            NetApiBufferFree(wki);

            return tmp;
        }

        return "?.?";
    }
#elif __linux__
    #include <sys/utsname.h>
    std::string internal_getLinuxOSName() {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            return (std::string) buffer.sysname;
        }

        return "?Linux?";
    }
    std::string internal_getLinuxOSVersion() {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            return (std::string) buffer.release;
        }

        return "?v?";
    }
#elif __APPLE__
    #include <sys/sysctl.h>
    std::string internal_getMacOSVersion() {
        char osversion[256];
        size_t size = sizeof(osversion);
        // Gets the Darwin kernel version
        if (sysctlbyname("kern.osrelease", osversion, &size, NULL, 0) == 0) {
            return (std::string) osversion;
        }

        return "?.?";
    }
#else
    // ???
#endif

namespace Common {
    namespace OS {
        // Compilation-specific
        std::string getBuildArch() {
            #if defined(__x86_64__) || defined(_M_X64)
                return "x86_64 (64-bit Intel/AMD)";
            #elif defined(__i386__) || defined(_M_IX86)
                return "x86 (32-bit Intel/AMD)";
            #elif defined(__aarch64__) || defined(_M_ARM64)
                return "ARM64 (64-bit ARM)";
            #elif defined(__arm__) || defined(_M_ARM)
                return "ARM (32-bit ARM)";
            #else
                return "Unknown Architecture";
            #endif
        }

        // System-specific
        std::string getOSName() {
            #if _WIN32
                return internal_getWindowsName();
            #elif __linux__
                return internal_getLinuxOSName();
            #elif __APPLE__
                return "macOS";
            #else
                return "Unknown OS";
            #endif
        }
        std::string getOSVersion() {
            #if _WIN32
                return internal_getWindowsVersion();
            #elif __linux__
                return internal_getLinuxOSVersion();
            #elif __APPLE__
                return internal_getMacOSVersion();
            #else
                return "?.?";
            #endif
        }
    }
}
