/**
 * @brief
 * Jug command wrapper
**/

// Basic C++
#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#elif __linux__
    #include <unistd.h>
    #include <limits.h>
#elif __APPLE__
    #include <mach-o/dyld.h>
    #include <limits.h>
#endif

std::filesystem::path getExecutableDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
#elif __linux__
    char buffer[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (count != -1) {
        return std::filesystem::path(std::string(buffer, count)).parent_path();
    }
    return "";
#elif __APPLE__
    char raw_path[PATH_MAX];
    uint32_t size = sizeof(raw_path);
    if (_NSGetExecutablePath(raw_path, &size) == 0) {
        char real_path[PATH_MAX];
        // resolve symlinks and '..' to get the absolute canonical path
        if (realpath(raw_path, real_path)) {
            return std::filesystem::path(real_path).parent_path();
        }
    }
    return "";
#else
    return ""; // Fallback for unsupported platforms
#endif
}

// Cross-platform process spawning helper
int spawn (const std::string& binary_name, int argc, char* argv[]) {
    std::filesystem::path exe_dir = getExecutableDirectory();

    if (exe_dir.empty()) {
        std::cerr << "Error: Could not determine executable location.\n";
        return 1;
    }

    std::filesystem::path target_exe = exe_dir / binary_name;

    #ifdef _WIN32
        target_exe.replace_extension(".exe");
    #endif

    if (!std::filesystem::exists(target_exe)) {
        std::cerr << "Error: Subcommand binary not found at " << target_exe << "\n";
        return 1;
    }

    // Reconstruct the command line forwarding argv[2] onwards
    std::string command = "\"" + target_exe.string() + "\"";
    for (int i = 2; i < argc; ++i) {
        command += " ";
        command += argv[i];
    }

    // Native execution call
    return std::system(command.c_str());
}

int main (int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Juggernyaut Toolchain\n\n"
                  << "Usage: jug [command] [options]\n\n"
                  << "Commands:\n"
                  << "    cmp    Launch the compiler\n"
                  << "    pck    Launch the package manager\n"
                  << "    lsp    Launch the language server\n";
        return 0;
    }

    std::string subcommand = argv[1];

    if (subcommand == "cmp") return spawn("JuggernyautCompiler", argc, argv);
    if (subcommand == "pck") return spawn("JuggernyautPackageManager", argc, argv);
    if (subcommand == "lsp") return spawn("JuggernyautServer", argc, argv);

    std::cerr << "Error: Unknown command '" << subcommand << "'\n";
    return 1;
}
