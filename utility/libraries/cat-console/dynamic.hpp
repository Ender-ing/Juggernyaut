/**
 * @brief 
 * Manage dynamic library exports and imports
**/

#pragma once

#ifdef _WIN32
#   ifdef JUG_CAT_CONSOLE_LIBRARY_EXPORTS // Exporting on Windows
#       define JUG_CAT_CONSOLE_API __declspec(dllexport)
#   else // Importing on Windows
#       define JUG_CAT_CONSOLE_API __declspec(dllimport)
#   endif
#else // Linux
#   define JUG_CAT_CONSOLE_API __attribute__((visibility("default")))
#endif
