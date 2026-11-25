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

    // Workflow & Lifecycle:
    // 1. Setup (`reset`):
    //    Binds the target `File` and `ImportRequest` sink, allocates a fresh `AST` (with node
    //    capacity pre-reserved from file byte length), and clears LIFO scratch stacks while
    //    retaining their heap capacities across parses on `thread_local` instances.
    //
    // 2. Pre-order (`onValidEnter` <- `CSTVisitor`):
    //    Looks up `SymbolRule` in `LoweringTable` in O(1) by `ts::Symbol`:
    //    - `EmitNode`: Reserves `AST::nodes[0]` if root, records `pendingStart` in
    //      `astPendingChildren`, runs `rule.onEnter(*this, ...)`, and pushes an `ASTFrame`.
    //    - `PassThrough`: Emits no frame; if `forwardField` is set, forwards its `ts::FieldId`
    //      to unwrapped children via `activeForwardedField` (restored via `forwardedFieldStack`).
    //    - `MutateParent`: Stamps `astKind`, `subKind`, and `flags` onto `astStack.back()`
    //      (guarded by `targetParent`) and runs `rule.onEnter`.
    //    - `Ignore` / `Unhandled`: Returns `true` (handled) or `false` (unhandled) with no AST node.
    //
    // 3. Error recovery (`onSyntaxError` <- `CSTVisitor`):
    //    Appends an isolated `ASTKind::Dirty` child inside `Program` / `Block` scopes so valid
    //    sibling statements are preserved, or marks any narrower enclosing `ASTFrame` as `Dirty`.
    //
    // 4. Post-order (`onValidLeave` -> `finalizeFrame`):
    //    Pops the `ASTFrame` and consumes its contiguous tail slice `astPendingChildren[pendingStart..]`:
    //    - Resolves child `ASTRole`s in O(1) via `LoweringTable::lookupRole` when `resolveChildRoles` is set.
    //    - Commits children contiguously into `AST::nodes` (`firstChild` + `childCount`) and truncates
    //      `astPendingChildren` back to `pendingStart`.
    //    - Runs `rule.onLeave(*this, node, frame.astNode)` (unless `Dirty`), where hooks can inspect
    //      finalized `childrenOf(astNode)`, slice `sourceText(node)`, or call `pushImport(...)`.
    //    - Stages `frame.astNode` into the parent's pending slice (or writes `reservedSlot` at the root).
    //
    // 5. Extraction (`takeAST`):
    //    Detaches per-file pointers, clears scratch stacks, and transfers ownership of the built `AST`.
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

            // Output extraction
            // (clears transient per-file pointers/stacks)
            [[nodiscard]] std::unique_ptr<AST> takeAST() noexcept;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
