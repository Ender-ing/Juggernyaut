/**
 * @brief 
 * Abstract Syntax Tree Node
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "../diagnostics/Diagnostic.hpp"
#include "../host/FileBuffer.hpp"

namespace Data::Parser {
    // Host
    using FileByte = Host::FileByte;

    // Diagnostics
    using Range = Diagnostics::Range;

    enum class ASTKind : uint16_t {
        // Technical
        None = 0,  // Transparent wrapper / unset
        Dirty = 1, // Unparsed / syntax-error placeholder span

        // Basic
        Program,
        Block,

        // Terminals & literals
        Identifier,
        StringLiteral,

        // Statements
        ImportStmt,
        ImportLibrary
    };

    // Semantic role this node occupies inside its parent ASTNode
    enum class ASTRole : uint16_t {
        None = 0, // Positional / unfielded child

        // Module & symbol relations
        Source, // Import target (`import_path` string literal or `import_library`)
        Symbol, // Discrete imported/exported symbol identifier (`import { a, b }`)
        Alias   // Encapsulated module handle (`import ... as handle`)
    };

    // Sub-kinds
    // `ASTNode::subKind` discriminators for `ASTKind::ImportStmt` (`0` = unset)
    namespace ImportSubKind {
        constexpr uint16_t None        = 0u;
        constexpr uint16_t Discrete    = 1u; // `import { ... } from ...;`
        constexpr uint16_t Encapsulate = 2u; // `import ... as ...;`
    }

    // Reserved & common bit-flags on `ASTNode::flags` (lower bits available per-ASTKind)
    namespace ASTFlags {
        constexpr uint16_t None     = 0u;
        constexpr uint16_t HasError = uint16_t{1u} << 15u; // Subtree contains syntax error(s); `kind` is preserved
    }

    // Per-node bit-flags for `ASTKind::ImportStmt`
    namespace ImportFlags {
        constexpr uint16_t External = uint16_t{1u} << 0u; // Library import (`import_library`) vs local file (`import_path`)
    }

    // Half-open UTF-8 byte span `[start, end)` within `File::content`
    struct ByteSpan {
        FileByte start = 0;
        FileByte end = 0;

        [[nodiscard]] constexpr FileByte length() const noexcept {
            return this->end >= this->start ? (this->end - this->start) : 0u;
        }
        constexpr bool operator==(const ByteSpan&) const noexcept = default;
    };

    using LexemeId = uint32_t;
    constexpr LexemeId INVALID_LEXEME_ID = ~LexemeId{0};

    // Final node (32B exact = 2 per 64B cache line: 8B header + 8B byte span + 8B child slice + 8B lexeme/payload)
    struct ASTNode {
        ASTKind kind = ASTKind::None;
        ASTRole role = ASTRole::None;
        uint16_t subKind = 0;
        uint16_t flags = ASTFlags::None;
        ByteSpan span{};

        // Location in vector
        uint32_t firstChild = 0;
        uint32_t childCount = 0;

        // (Source) data
        LexemeId lexemeId = INVALID_LEXEME_ID;
        uint32_t payload = 0;

        [[nodiscard]] constexpr bool hasError() const noexcept {
            return (this->flags & ASTFlags::HasError) != 0u || this->kind == ASTKind::Dirty;
        }
    };
    static_assert(sizeof(ASTNode) == 32, "ASTNode must remain 32 bytes (2 per cache line)");
}
