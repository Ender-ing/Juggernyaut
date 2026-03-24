/**
 * @brief 
 * Commnly used headers
**/

#pragma once

// Basic configurations
// #pragma execution_character_set("utf-8") (NOT STANDARD)

// Basic C++ headers
//#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <limits.h>
#include <optional>

// Include platform headers
#ifdef _WIN32
    #include <Windows.h>
    #include <libloaderapi.h> // GetModuleFileNameW, ...
#elif __linux__ // Linux
    #include <unistd.h> // readlink, ...
#elif __APPLE__ // macOS (and other Apple platforms)
    #include <mach/mach_time.h>
    #include <mach-o/dyld.h> // _NSGetExecutablePath, ...
//#elif __EMSCRIPTEN__ // WASM (maybe? It'd be useful but it requires a lot of effort to implement)
//    #include <emscripten.h>
#else
    // ???
#endif

// TMP
#define BAD_CODE_OR_MEMORY_LEAKS "\nPossible memory leaks/bad code. Please contact the developers!"
