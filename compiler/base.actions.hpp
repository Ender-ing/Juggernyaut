/**
 * @brief
 * Manage compiler actions
**/

#pragma once

#include "common/headers.hpp"
#include "base.dynamic.hpp" // JUG_BASE_API

// Basic C++ headers
#include <unordered_map>
#include <functional>
#include <optional>

namespace Base {
    namespace Actions {
        // Action-related function types
        typedef std::function<bool(std::string&, bool)> ActionNextFunction;
        typedef std::array<std::string, 3> ActionInfo;
        typedef std::vector<std::string> ActionInput; // Change as needed!
        typedef std::function<bool(const ActionNextFunction)> ActionFunction;
        struct Action {
            ActionInfo info;
            ActionInput input;
            ActionFunction func;
        };

        // List of actions and their respective functions
        // [
        //  string[
        //      // flag name members must be in lower case and only contain english letters and dashes (-)
        //      string (short flag name)
        //      string (long flag name)
        //      string (flag/action description)
        //  ]
        //  string[
        //      (string (<input_name> - option_1 (...) | ...)))*
        //  ]
        //  function (true - normal -> continue, false - error -> terminate) // Must always return a boolean value
        // ]
        typedef std::vector<Action> ActionsList; // Change the array size as needed!
        extern JUG_BASE_API const ActionsList actions;

        // Get an action function using one flag
        extern JUG_BASE_API bool getActionFunctionByFlag(const std::string& flag, ActionFunction &store) ;
    }
}
