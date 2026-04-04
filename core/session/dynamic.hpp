/**
 * @brief 
 * Manage dynamic library exports and imports
**/

#pragma once

#ifdef _WIN32
#   ifdef JUG_SESSION_LIBRARY_EXPORTS // Exporting on Windows
#       define JUG_SESSION_API __declspec(dllexport)
#   else // Importing on Windows
#       define JUG_SESSION_API __declspec(dllimport)
#   endif
#else // Linux
#   define JUG_SESSION_API __attribute__((visibility("default")))
#endif
