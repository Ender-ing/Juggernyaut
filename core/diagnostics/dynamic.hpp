/// @file
/// @ingroup CoreDiagnostics
/// @brief Defines shared library export/import visibility macros for the Diagnostics library.

#pragma once

/// @addtogroup CoreDiagnostics
/// @{

#ifdef _WIN32
#   ifdef JUG_DIAGNOSTICS_LIBRARY_EXPORTS // Exporting on Windows
#       define JUG_DIAGNOSTICS_API __declspec(dllexport)
#   else // Importing on Windows
#       define JUG_DIAGNOSTICS_API __declspec(dllimport)
#   endif
#else // Linux
    /// @brief Symbol visibility macro for functions and types exported by `JuggernyautDiagnosticsLibrary`.
#   define JUG_DIAGNOSTICS_API __attribute__((visibility("default")))
#endif

/// @}
