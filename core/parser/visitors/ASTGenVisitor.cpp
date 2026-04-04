/**
 * @brief
 * Session Workflow AST generator
**/

#include "ASTGenVisitor.hpp"

#define DEFAULT_VISIT \
{ return this->visitChildren(context); }

namespace Parser {
    namespace Visitors {
        std::any ASTGenVisitor::visitProg(ANTLRParser::ProgContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitStatement(ANTLRParser::StatementContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitStat_import(ANTLRParser::Stat_importContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitImport_discrete(ANTLRParser::Import_discreteContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitImport_encapsulate(ANTLRParser::Import_encapsulateContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitImport_list(ANTLRParser::Import_listContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitImport_source(ANTLRParser::Import_sourceContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitImport_path(ANTLRParser::Import_pathContext *context) {
            auto token = context->LIT_STRING();
            if (token != nullptr) {
                // ...
            }
            return nullptr;
        }
        std::any ASTGenVisitor::visitImport_library(ANTLRParser::Import_libraryContext *context) {
            Diagnostics::Diagnostic diag = Listeners::Internal::getGenRuleDiagnostic(context);

            // auto tokens = context->IDENTIFIER();

            // Update diagnostic data
            diag.code = 1;
            diag.message = "library imports have yet to be implemented (temporary)";
            diag.severity = Diagnostics::Severity::Warning;

            // Attach diagnostic to source
            this->listener->pushDiagnostic(diag);

            return nullptr;
        }
        std::any ASTGenVisitor::visitStat_expr(ANTLRParser::Stat_exprContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitStat_empty(ANTLRParser::Stat_emptyContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitExpr(ANTLRParser::ExprContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitPrim(ANTLRParser::PrimContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitIdentifier(ANTLRParser::IdentifierContext *context) DEFAULT_VISIT
        std::any ASTGenVisitor::visitLit_string(ANTLRParser::Lit_stringContext *context) DEFAULT_VISIT
    }
}
