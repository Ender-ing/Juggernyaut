/**
 * @brief
 * AST & ImportRequest generator visitor
**/

#include "ASTGenVisitor.hpp"

// Diagnostics helpers
#include "../../diagnostics/astGen.hpp"

#define DEFAULT_VISIT \
    { return this->visitChildren(context); }

namespace Parser::Visitors {
    void ASTGenVisitor::emitDiagnostic(Diagnostics::Diagnostic &diag) {
        diag.file = this->file;
        if (this->onDiagnostic != nullptr) {
            this->onDiagnostic(diag);
        }
    }

    ImportRequest ASTGenVisitor::extractImportSource(ANTLRParser::Import_sourceContext *context) {
        ImportRequest req;
        if (context == nullptr) {
            return req;
        }

        req.range = Diagnostics::getGenRuleDiagnostic(context).range;

        if (auto *pathCtx = context->import_path()) {
            req.isExternal = false;

            if (auto *token = pathCtx->LIT_STRING()) {
                std::string raw = token->getText(); // ...maybe this should converted into canonical form?
                if (raw.size() >= 2) {
                    req.target = raw.substr(1, raw.size() - 2);
                }
            }
        } else if (auto *libCtx = context->import_library()) {
            req.isExternal = true;

            const auto identifiers = libCtx->IDENTIFIER();
            if (identifiers.size() == 1) {
                req.target = identifiers[0]->getText();
            } else if (identifiers.size() >= 2) {
                req.target = identifiers[0]->getText() + "." + identifiers[1]->getText();
            }
        }

        return req;
    }

    std::any ASTGenVisitor::visitProg(ANTLRParser::ProgContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStatement(ANTLRParser::StatementContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_import(ANTLRParser::Stat_importContext *context) DEFAULT_VISIT

    std::any ASTGenVisitor::visitImport_discrete(ANTLRParser::Import_discreteContext *context) {
        ImportRequest req = this->extractImportSource(context->import_source());

        if (auto *listCtx = context->import_list()) {
            const auto identifiers = listCtx->identifier();
            req.symbols.reserve(identifiers.size());
            for (auto *idCtx : identifiers) {
                req.symbols.push_back(idCtx->getText());
            }
        }

        if (!req.target.empty()) {
            this->result.imports.push_back(std::move(req));
        }
        return nullptr;
    }

    std::any ASTGenVisitor::visitImport_encapsulate(ANTLRParser::Import_encapsulateContext *context) {
        ImportRequest req = this->extractImportSource(context->import_source());

        if (auto *aliasToken = context->IDENTIFIER()) {
            req.alias = aliasToken->getText();
        }

        if (!req.target.empty()) {
            this->result.imports.push_back(std::move(req));
        }
        return nullptr;
    }

    std::any ASTGenVisitor::visitImport_list(ANTLRParser::Import_listContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitImport_source(ANTLRParser::Import_sourceContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitImport_path(ANTLRParser::Import_pathContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitImport_library(ANTLRParser::Import_libraryContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_expr(ANTLRParser::Stat_exprContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_empty(ANTLRParser::Stat_emptyContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExpr(ANTLRParser::ExprContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitPrim(ANTLRParser::PrimContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitIdentifier(ANTLRParser::IdentifierContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitLit_string(ANTLRParser::Lit_stringContext *context) DEFAULT_VISIT
}
