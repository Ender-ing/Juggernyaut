/**
 * @brief
 * Jug `import` statement lowering rules & dependency extraction
**/

#include "import.hpp"

// Lowering & literals
#include "../../lowering/ASTBuilder.hpp"
#include "../base/literals.hpp"

namespace Parser::Rules::Statements {
    namespace Helper {
        void emitImportRequest(Lowering::ASTBuilder &builder, ts::Node, Data::Parser::ASTNode &astNode) {
            using namespace Data::Parser;

            AST *ast = builder.currentAST();
            const Data::Host::File *file = builder.currentFile();
            if (ast == nullptr || file == nullptr) {
                return;
            }

            ImportRequest req{};
            const ASTNode *sourceNode = nullptr;

            for (const ASTNode &child : builder.childrenOf(astNode)) {
                switch (child.role) {
                    case ASTRole::Source: {
                        sourceNode = &child;
                        if (child.kind == ASTKind::ImportLibrary) {
                            req.isExternal = true;
                            astNode.flags = static_cast<uint16_t>(astNode.flags | ImportFlags::External);
                            for (const ASTNode &part : builder.childrenOf(child)) {
                                if (part.kind != ASTKind::Identifier) {
                                    continue;
                                }
                                if (!req.target.empty()) {
                                    req.target.push_back('.');
                                }
                                req.target.append(ast->lexemeText(part.lexemeId));
                            }
                        } else if (child.kind == ASTKind::StringLiteral) {
                            req.isExternal = false;
                            req.target = Base::unescapeStringLiteral(ast->lexemeText(child.lexemeId));
                        }
                        break;
                    }
                    case ASTRole::Symbol: {
                        if (child.kind == ASTKind::Identifier) {
                            req.symbols.emplace_back(ast->lexemeText(child.lexemeId));
                        }
                        break;
                    }
                    case ASTRole::Alias: {
                        if (child.kind == ASTKind::Identifier) {
                            req.alias = std::string(ast->lexemeText(child.lexemeId));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            if (sourceNode == nullptr) {
                return;
            }

            req.range = ast->rangeOf(*sourceNode, *file);
            builder.pushImport(std::move(req));
        }
    }

    void registerImport(Lowering::LoweringTable &table) {
        using namespace Data::Parser;

        table.bindPassThrough("stat_import");
        table.bindPassThrough("import_list", true);
        table.bindPassThrough("import_source", true);
        table.bindPassThrough("import_path", true);
        table.bindNode("import_library", ASTKind::ImportLibrary);

        table.bindNode("import_discrete", Lowering::SymbolRule{
            .op = Lowering::LoweringOp::EmitNode,
            .astKind = ASTKind::ImportStmt,
            .subKind = ImportSubKind::Discrete,
            .resolveChildRoles = true,
            .onLeave = &Helper::emitImportRequest,
        });
        table.bindNode("import_encapsulate", Lowering::SymbolRule{
            .op = Lowering::LoweringOp::EmitNode,
            .astKind = ASTKind::ImportStmt,
            .subKind = ImportSubKind::Encapsulate,
            .resolveChildRoles = true,
            .onLeave = &Helper::emitImportRequest,
        });

        table.bindScopedRole(ASTKind::ImportStmt, "source", ASTRole::Source);
        table.bindScopedRole(ASTKind::ImportStmt, "list", ASTRole::Symbol);
        table.bindScopedRole(ASTKind::ImportStmt, "handle", ASTRole::Alias);
    }
}
