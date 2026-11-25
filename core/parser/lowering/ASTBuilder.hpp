/**
 * @brief
 * Stateful per-parse CSTConsumer that assembles flat ASTs using a LoweringTable
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// Tree-sitter
#include "../tree-sitter.hpp"

// Host, AST & Parser types
#include "../../data/host/VFS.hpp"
#include "../../data/parser/AST.hpp"
#include "../Parser.hpp"

// Lowering spec
#include "LoweringTable.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Parser::Lowering {
    using File = Data::Host::File;
    using AST = Data::Parser::AST;

    class JUG_PARSER_API ASTBuilder {
        private:
            // Active parent AST node under construction (48B exact, zero padding, trivially copyable)
            struct ASTFrame {
                static constexpr uint32_t NO_SLOT = ~uint32_t{0};

                ts::NodeID nodeId{};
                ASTNode astNode{};
                uint32_t pendingStart = 0;
                uint32_t reservedSlot = NO_SLOT;
                ts::FieldId cstFieldId = ts::FieldId::None;
                ts::FieldId activeForwardedField = ts::FieldId::None;
            };

            // Completed child node staged in `astPendingChildren` (32B, 2 per cache line)
            struct PendingASTChild {
                ASTNode astNode{};
                ts::FieldId cstFieldId = ts::FieldId::None;
            };

            // Saved field state when a `PassThrough` wrapper alters `activeForwardedField` (16B)
            struct ForwardedFieldFrame {
                ts::NodeID nodeId{};
                ts::FieldId prevField = ts::FieldId::None;
            };

            const LoweringTable *table = nullptr;
            const File *file = nullptr;
            std::vector<ImportRequest> *imports = nullptr;
            std::unique_ptr<AST> ast = nullptr;

            // Reusable LIFO scratch buffers (capacities stay warm on thread_local instances)
            std::vector<ASTFrame> astStack;
            std::vector<PendingASTChild> astPendingChildren;
            std::vector<ForwardedFieldFrame> forwardedFieldStack;

            void clearScratch() noexcept;
            void finalizeFrame(ts::Node node, const SymbolRule &rule);

        public:
            explicit ASTBuilder(const LoweringTable *loweringTable = nullptr) noexcept;
            ~ASTBuilder() = default;

            ASTBuilder(const ASTBuilder&) = delete;
            ASTBuilder& operator=(const ASTBuilder&) = delete;
            ASTBuilder(ASTBuilder&&) noexcept = default;
            ASTBuilder& operator=(ASTBuilder&&) noexcept = default;

            // Prepare builder for a new file parse
            void reset(const File *targetFile = nullptr, std::vector<ImportRequest> *importSink = nullptr) ;

            // `Visitors::CSTConsumer` interface
            [[nodiscard]] bool onValidEnter(ts::Node node, ts::FieldId fieldId) ;
            void onValidLeave(ts::Node node) ;
            void onSyntaxError(ts::Node errorNode) ;

            // Context helpers exposed to `SymbolRule::Hook`
            [[nodiscard]] const File* currentFile() const noexcept;
            [[nodiscard]] AST* currentAST() noexcept;
            [[nodiscard]] std::string_view sourceText(ts::Node node) const noexcept;
            [[nodiscard]] std::span<ASTNode> childrenOf(const ASTNode &parent) noexcept;
            void pushImport(ImportRequest &&request) ;

            // Output extraction (clears transient per-file pointers/stacks)
            [[nodiscard]] std::unique_ptr<AST> takeAST() noexcept;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
