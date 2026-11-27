/**
 * @brief 
 * Parser Abstract Syntax Tree
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

// Diagnostics
#include "../diagnostics/Diagnostic.hpp"

// Host
#include "../host/File.hpp"

// AST
#include "ASTNode.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Data::Parser {
    // Formatting options for `AST::toSExpr`
    struct ASTDumpOptions {
        bool pretty = false;        // Multi-line indented tree vs single-line S-expression
        bool includeSpans = false;  // Emit `@start..end` byte offsets on every node (`Dirty` always emits spans)
        bool includeLexemes = true; // Emit escaped `"..."` text for nodes with an interned `lexemeId`
        uint8_t indentSpaces = 2;   // Spaces per nesting level when `pretty == true`
        size_t maxBytes = 0;        // `0` = unlimited; otherwise stops cleanly once output reaches `maxBytes`
    };

    struct JUG_DATA_API AST {
        std::vector<ASTNode> nodes;
        std::vector<std::string> lexemes;

        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] const ASTNode& root() const noexcept;

        [[nodiscard]] std::span<const ASTNode> children(const ASTNode &target) const noexcept;
        [[nodiscard]] const ASTNode* childByRole(const ASTNode &parent, ASTRole role) const noexcept;

        // Appends a unique lexeme (deduplication is handled in O(1) by `ASTBuilder`'s scratch map)
        [[nodiscard]] LexemeId pushLexeme(std::string_view text) ;
        [[nodiscard]] std::string_view lexemeText(LexemeId id) const noexcept;
        [[nodiscard]] size_t internedLexemeCount() const noexcept;

        // Source text & diagnostic range resolution from compact `ByteSpan`
        [[nodiscard]] std::string_view textOf(const ASTNode &target, const Host::File &sourceFile) const noexcept;
        [[nodiscard]] Diagnostics::Range rangeOf(const ASTNode &target, const Host::File &sourceFile) const noexcept;

        // Iterative S-expression debug dump
        [[nodiscard]] std::string toSExpr(ASTDumpOptions options = {}, uint32_t rootIndex = 0u) const ;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
