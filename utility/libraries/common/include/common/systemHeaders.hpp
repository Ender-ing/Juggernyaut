/**
 * @brief 
 * Thread headers
**/

#pragma once

// Basic C++ headers
#if (!defined(__EMSCRIPTEN__) && !defined(__wasi__)) || defined(__EMSCRIPTEN_PTHREADS__) || defined(_REENTRANT)
    #define JUG_HAS_THREADS 1
    #include <thread>
#else
    #define JUG_HAS_THREADS 0
#endif
