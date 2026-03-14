/**
 * @brief 
 * Manage dynamic library exports and imports
**/

#pragma once

#ifdef _WIN32
#   ifdef JUG_BASE_LIBRARY_EXPORTS // Exporting on Windows
#       define JUG_BASE_API __declspec(dllexport)
#   else // Importing on Windows
#       define JUG_BASE_API __declspec(dllimport)
#   endif
#else // Linux & macOS
#   define JUG_BASE_API __attribute__((visibility("default")))
#endif
