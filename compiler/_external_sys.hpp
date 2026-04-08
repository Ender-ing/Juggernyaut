/**
 * @brief 
 * Commonly used system headers (not actually part of the common library)
**/

#pragma once

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
