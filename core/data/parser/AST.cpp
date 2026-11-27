/**
 * @brief 
 * Parser Abstract Syntax Tree
**/

#include "AST.hpp"

// C++ headers
#include <algorithm>
#include <charconv>

namespace Data::Parser {
    namespace Helper {
        constexpr size_t MAX_INITIAL_DUMP_RESERVE = 4u * 1024u * 1024u; // 4 MB initial reserve cap

        // Internal iterative walker (`onEnter` returns `false` to abort early)
        template <typename EnterFn, typename LeaveFn>
        void walkAST(const AST &ast, uint32_t rootIndex, EnterFn &&onEnter, LeaveFn &&onLeave) {
            const size_t totalNodes = ast.nodes.size();
            if (rootIndex >= totalNodes) {
                return;
            }

            static constexpr uint32_t UNENTERED = ~uint32_t{0};
            struct Frame {
                uint32_t nodeIdx = 0;
                uint32_t nextChild = UNENTERED;
                uint32_t depth = 0;
            };

            std::vector<Frame> stack;
            stack.reserve(64u);
            stack.push_back(Frame{.nodeIdx = rootIndex, .nextChild = UNENTERED, .depth = 0u});

            size_t visitedCount = 0u;

            while (!stack.empty()) {
                Frame &top = stack.back();
                const ASTNode &node = ast.nodes[top.nodeIdx];

                if (top.nextChild == UNENTERED) {
                    if (++visitedCount > totalNodes || !onEnter(node, top.depth)) {
                        return;
                    }

                    const bool hasChildren = (node.childCount > 0u) &&
                        ((static_cast<size_t>(node.firstChild) + node.childCount) <= totalNodes);
                    if (hasChildren) {
                        top.nextChild = 1u;
                        stack.push_back(Frame{
                            .nodeIdx = node.firstChild,
                            .nextChild = UNENTERED,
                            .depth = top.depth + 1u,
                        });
                    } else {
                        onLeave(node);
                        stack.pop_back();
                    }
                } else if (top.nextChild < node.childCount) {
                    const uint32_t childIdx = node.firstChild + top.nextChild;
                    ++top.nextChild;
                    stack.push_back(Frame{
                        .nodeIdx = childIdx,
                        .nextChild = UNENTERED,
                        .depth = top.depth + 1u,
                    });
                } else {
                    onLeave(node);
                    stack.pop_back();
                }
            }
        }

        void appendUint32(std::string &out, uint32_t value) {
            char buf[16];
            const auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value);
            if (ec == std::errc{}) {
                out.append(buf, static_cast<size_t>(ptr - buf));
            }
        }
        void appendHex16(std::string &out, uint16_t value) {
            char buf[8];
            out.append("0x", 2);
            const auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value, 16);
            if (ec == std::errc{}) {
                out.append(buf, static_cast<size_t>(ptr - buf));
            }
        }
        void appendEscapedLexeme(std::string &out, std::string_view text) {
            static constexpr char HEX_DIGITS[] = "0123456789ABCDEF";
            out.push_back('"');

            size_t runStart = 0;
            for (size_t i = 0; i < text.size(); ++i) {
                const auto ch = static_cast<unsigned char>(text[i]);
                std::string_view replacement;
                char hexEsc[4] = {'\\', 'x', '0', '0'};

                switch (ch) {
                    case '"':  replacement = "\\\""; break;
                    case '\\': replacement = "\\\\"; break;
                    case '\n': replacement = "\\n";  break;
                    case '\r': replacement = "\\r";  break;
                    case '\t': replacement = "\\t";  break;
                    case '\0': replacement = "\\0";  break;
                    default:
                        if (ch < 0x20u || ch == 0x7Fu) {
                            hexEsc[2] = HEX_DIGITS[(ch >> 4u) & 0x0Fu];
                            hexEsc[3] = HEX_DIGITS[ch & 0x0Fu];
                            replacement = std::string_view(hexEsc, 4);
                        }
                        break;
                }

                if (!replacement.empty()) {
                    if (i > runStart) {
                        out.append(text.data() + runStart, i - runStart);
                    }
                    out.append(replacement);
                    runStart = i + 1u;
                }
            }

            if (runStart < text.size()) {
                out.append(text.data() + runStart, text.size() - runStart);
            }
            out.push_back('"');
        }
        void appendFlags(std::string &out, ASTKind kind, uint16_t flags) {
            uint16_t bits = flags;
            if (kind == ASTKind::Dirty) {
                bits = static_cast<uint16_t>(bits & ~ASTFlags::HasError);
            }
            if (bits == ASTFlags::None) {
                return;
            }

            out.append(" [", 2);
            bool first = true;
            uint16_t unknownBits = 0u;

            // Iterate only over set bits in ascending bit order
            while (bits != 0u) {
                const auto lowestBit = static_cast<uint16_t>(
                    bits & static_cast<uint16_t>(static_cast<uint16_t>(~bits) + 1u)
                );
                bits = static_cast<uint16_t>(bits & (bits - 1u));

                const std::string_view name = astFlagName(kind, lowestBit);
                if (!name.empty()) {
                    if (!first) {
                        out.push_back(',');
                    }
                    out.append(name);
                    first = false;
                } else {
                    unknownBits = static_cast<uint16_t>(unknownBits | lowestBit);
                }
            }

            if (unknownBits != 0u) {
                if (!first) {
                    out.push_back(',');
                }
                appendHex16(out, unknownBits);
            }
            out.push_back(']');
        }
    }

    [[nodiscard]] bool AST::empty() const noexcept {
        return this->nodes.empty();
    }
    [[nodiscard]] const ASTNode& AST::root() const noexcept {
        return this->nodes.front();
    }

    [[nodiscard]] std::span<const ASTNode> AST::children(const ASTNode &target) const noexcept {
        if (target.childCount == 0u ||
            (static_cast<size_t>(target.firstChild) + target.childCount) > this->nodes.size()) {
            return {};
        }
        return std::span<const ASTNode>(this->nodes.data() + target.firstChild, target.childCount);
    }
    [[nodiscard]] const ASTNode* AST::childByRole(const ASTNode &parent, ASTRole role) const noexcept {
        for (const ASTNode &child : this->children(parent)) {
            if (child.role == role) {
                return &child;
            }
        }
        return nullptr;
    }

    [[nodiscard]] LexemeId AST::pushLexeme(std::string_view text) {
        const auto id = static_cast<LexemeId>(this->lexemes.size());
        this->lexemes.emplace_back(text);
        return id;
    }
    [[nodiscard]] std::string_view AST::lexemeText(LexemeId id) const noexcept {
        return id < this->lexemes.size() ? std::string_view(this->lexemes[id]) : std::string_view{};
    }
    [[nodiscard]] size_t AST::internedLexemeCount() const noexcept {
        return this->lexemes.size();
    }
    [[nodiscard]] std::string_view AST::textOf(const ASTNode &target, const Host::File &sourceFile) const noexcept {
        const std::string_view src = sourceFile.content.view();
        if (target.span.start >= src.size() || target.span.end <= target.span.start) {
            return {};
        }
        const size_t len = std::min<size_t>(target.span.length(), src.size() - target.span.start);
        return src.substr(target.span.start, len);
    }
    [[nodiscard]] Diagnostics::Range AST::rangeOf(const ASTNode &target, const Host::File &sourceFile) const noexcept {
        const Host::TextPoint startPt = sourceFile.content.pointAtByte(target.span.start);
        const Host::TextPoint endPt = sourceFile.content.pointAtByte(target.span.end);
        return Diagnostics::Range{
            .start = Diagnostics::Position{.line = startPt.line, .character = startPt.column},
            .end = Diagnostics::Position{.line = endPt.line, .character = endPt.column},
        };
    }

    // Iterative S-expression debug dump
    [[nodiscard]] std::string AST::toSExpr(ASTDumpOptions options, uint32_t rootIndex) const {
        if (rootIndex >= this->nodes.size()) {
            return "()";
        }

        const size_t estimatedBytes = this->nodes.size() * (options.pretty ? 32u : 24u);
        const size_t reserveCap = options.maxBytes > 0u
            ? std::min(options.maxBytes + 32u, Helper::MAX_INITIAL_DUMP_RESERVE)
            : std::min(estimatedBytes, Helper::MAX_INITIAL_DUMP_RESERVE);

        std::string out;
        out.reserve(reserveCap);

        Helper::walkAST(
            *this,
            rootIndex,
            [&](const ASTNode &node, uint32_t depth) -> bool {
                if (options.maxBytes > 0u && out.size() >= options.maxBytes) {
                    out.append(" ... <truncated>");
                    return false;
                }

                if (depth > 0u) {
                    if (options.pretty) {
                        out.push_back('\n');
                        out.append(static_cast<size_t>(depth) * options.indentSpaces, ' ');
                    } else {
                        out.push_back(' ');
                    }

                    if (node.role != ASTRole::None) {
                        out.append(astRoleName(node.role));
                        out.append(": ", 2);
                    }
                }

                out.push_back('(');
                out.append(astKindName(node.kind));
                if (node.subKind != 0u) {
                    out.push_back('.');
                    const std::string_view subName = astSubKindName(node.kind, node.subKind);
                    if (!subName.empty()) {
                        out.append(subName);
                    } else {
                        Helper::appendUint32(out, node.subKind);
                    }
                }

                Helper::appendFlags(out, node.kind, node.flags);

                if (options.includeSpans || node.kind == ASTKind::Dirty) {
                    out.append(" @", 2);
                    Helper::appendUint32(out, node.span.start);
                    out.append("..", 2);
                    Helper::appendUint32(out, node.span.end);
                }

                if (options.includeLexemes && node.lexemeId != INVALID_LEXEME_ID) {
                    out.push_back(' ');
                    Helper::appendEscapedLexeme(out, this->lexemeText(node.lexemeId));
                }

                return true;
            },
            [&](const ASTNode&) {
                out.push_back(')');
            }
        );

        return out;
    }
}
