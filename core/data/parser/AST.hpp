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
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
