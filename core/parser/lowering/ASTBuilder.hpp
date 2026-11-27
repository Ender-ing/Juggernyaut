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
    //    Binds the target `File`, `ImportRequest` sink, optional `LoweringTable` override, and
    //    per-parse `userContext`; allocates a fresh `AST` (pre-reserved at ~1 node per 8 bytes);
    //    and clears LIFO scratch stacks while retaining warm capacities on `thread_local` instances.
    //
    // 2. Pre-order (`onValidEnter` <- `CSTVisitor`):
    //    Resolves `SymbolRule` in `LoweringTable` by `(ts::Symbol, cstParent, astParent)` and caches
    //    `ActiveCSTEntry` on `cstStack` (so `onValidLeave` never re-queries Tree-sitter or the table):
    //    - `EmitNode`: Reserves `AST::nodes[0]` if root, claims any staged sibling-prefix modifiers
    //      (`MutateNext`) and prefix nodes (`attachToNext`), optionally interns `ASTNode::lexemeId`,
    //      runs `rule.onEnter(*this, ...)`, and pushes an `ASTFrame`.
    //    - `PassThrough`: Emits no frame; updates `activeForwardedField` (saved on `forwardedFieldStack`).
    //    - `MutateParent`: Stamps `astKind`, `subKind` (respecting `clobberSubKind`), and `flags` onto
    //      `astStack.back()` (guarded by `targetParent`) and runs `rule.onEnter`.
    //    - `MutateNext`: Stages `subKind` and `flags` on `astStack.back()` for the next sibling `EmitNode`.
    //    - `Ignore` / `Unhandled`: Returns `true` (handled) or `false` (unhandled; `onValidLeave` skipped).
    //
    // 3. Error recovery (`onSyntaxError` <- `CSTVisitor`):
    //    Sets `ASTFlags::HasError` on `astStack.back()` while preserving its `ASTKind` (so valid
    //    children/bodies remain usable by IDE/semantic passes), clears unconsumed prefix staging,
    //    and appends an isolated `ASTKind::Dirty` child covering the broken span.
    //
    // 4. Post-order (`onValidLeave` -> `finalizeFrame`):
    //    Pops `ActiveCSTEntry` and `ASTFrame`, resolves child `ASTRole`s over `astPendingFields`,
    //    bulk-inserts `astPendingChildren[pendingStart..]` into `AST::nodes` (`firstChild` + `childCount`),
    //    runs `rule.onLeave` (unless `hasError()` and `!runHooksOnError`), and stages `frame.astNode`
    //    into the parent's pending slice (marking `prefixStart` if `attachToNext` is set).
    //
    // 5. Extraction (`takeAST`):
    //    Detaches per-file pointers, trims any scratch buffers that exceeded high-water caps, and
    //    transfers ownership of the built `AST`.
    class JUG_PARSER_API ASTBuilder {
        private:
            // Cached CST ancestor entry (16B on x64; avoids re-querying `getSymbol` + `lookupSymbol` on leave)
            struct ActiveCSTEntry {
                ts::Symbol symbol{};
                const SymbolRule *rule = nullptr;
            };

            // Active parent AST node under construction
            struct ASTFrame {
                static constexpr uint32_t NO_SLOT = ~uint32_t{0};

                ts::NodeID nodeId{};
                ASTNode astNode{};
                uint32_t pendingStart = 0;
                uint32_t prefixStart = NO_SLOT;
                uint32_t reservedSlot = NO_SLOT;
                uint16_t pendingNextSubKind = 0;
                uint16_t pendingNextFlags = 0;
                ts::FieldId cstFieldId = ts::FieldId::None;
                ts::FieldId activeForwardedField = ts::FieldId::None;
            };

            // Saved field state when a `PassThrough` wrapper alters `activeForwardedField` (16B)
            struct ForwardedFieldFrame {
                ts::NodeID nodeId{};
                ts::FieldId prevField = ts::FieldId::None;
            };

            const LoweringTable *table = nullptr;
            const File *file = nullptr;
            std::vector<ImportRequest> *imports = nullptr;
            void *userContext = nullptr;
            std::unique_ptr<AST> ast = nullptr;

            // Reusable LIFO SoA scratch buffers (capacities stay warm on thread_local instances up to a cap)
            std::vector<ActiveCSTEntry> cstStack;
            std::vector<ASTFrame> astStack;
            std::vector<ASTNode> astPendingChildren;
            std::vector<ts::FieldId> astPendingFields;
            std::vector<ForwardedFieldFrame> forwardedFieldStack;

            void clearScratch(bool trimExcessCapacity) noexcept;
            void finalizeFrame(ts::Node node, const SymbolRule &rule) ;

        public:
            explicit ASTBuilder(const LoweringTable *loweringTable = nullptr) noexcept;
            ~ASTBuilder() = default;

            ASTBuilder(const ASTBuilder&) = delete;
            ASTBuilder& operator=(const ASTBuilder&) = delete;
            ASTBuilder(ASTBuilder&&) noexcept = default;
            ASTBuilder& operator=(ASTBuilder&&) noexcept = default;

            void setTable(const LoweringTable *loweringTable) noexcept;
            [[nodiscard]] const LoweringTable* getTable() const noexcept;

            // Prepare builder for a new file parse
            void reset(const File *targetFile = nullptr, std::vector<ImportRequest> *importSink = nullptr,
                const LoweringTable *overrideTable = nullptr, void *perParseUserContext = nullptr) ;

            // `Visitors::CSTConsumer` interface
            [[nodiscard]] bool onValidEnter(ts::Node node, ts::FieldId fieldId) ;
            void onValidLeave(ts::Node node) ;
            void onSyntaxError(ts::Node errorNode) ;

            // Context helpers exposed to `SymbolRule::Hook`
            [[nodiscard]] const File* currentFile() const noexcept;
            [[nodiscard]] AST* currentAST() noexcept;
            [[nodiscard]] std::string_view sourceText(ts::Node node) const noexcept;
            // Note: Returned span borrows `AST::nodes` and is invalidated if `AST::nodes` reallocates
            [[nodiscard]] std::span<const ASTNode> childrenOf(const ASTNode &parent) const noexcept;
            [[nodiscard]] ASTNode* mutableChildAt(const ASTNode &parent, uint32_t index) noexcept;
            void pushImport(ImportRequest &&request) ;

            template <typename T>
            [[nodiscard]] T* contextAs() noexcept {
                return static_cast<T*>(this->userContext);
            }

            // Output extraction (clears transient pointers & trims oversized scratch buffers)
            [[nodiscard]] std::unique_ptr<AST> takeAST() noexcept;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
