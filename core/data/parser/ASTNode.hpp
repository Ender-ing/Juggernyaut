/**
 * @brief 
 * Abstract Syntax Tree Node
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "./../diagnostics/Diagnostic.hpp"

namespace Data::Parser {
    using Range = Diagnostics::Range;

    enum class ASTKind : uint8_t {
        // Technical
        None = 0,  // Transparent wrapper
        Dirty = 1, // A contaminated node (original type is stripped)

        // Basic
        Program,
        Block
    };

    // Semantic role this node occupies inside its parent ASTNode
    enum class ASTRole : uint8_t {
        None = 0 // Positional / unfielded child
    };

    // Final node (28B exact: 4B header + 16B range + 8B child slice)
    struct ASTNode {
        ASTKind kind = ASTKind::None;
        ASTRole role = ASTRole::None;
        uint8_t subKind = 0;
        uint8_t flags = 0;
        Range range{};
        uint32_t firstChild = 0;
        uint32_t childCount = 0;
    };
}
