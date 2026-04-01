/**
 * @brief 
 * parser initiation
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_SESSION_API

#include "../session.hpp"

namespace Session {
    namespace Init {
        extern JUG_SESSION_API void parser(const Configs &configs, const Hooks &hooks) ;
    }
}
