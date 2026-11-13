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
                std::string raw = token->getText(); // This will be processed later!
                if (raw.size() >= 2) {
                    req.target = raw.substr(1, raw.size() - 2);
                }
            }
        } else if (auto *libCtx = context->import_library()) {
            req.isExternal = true;

            const auto identifiers = libCtx->identifier();
            for (size_t i = 0; i < identifiers.size(); ++i) {
                if (i > 0) {
                    req.target += ".";
                }
                req.target += identifiers[i]->getText();
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

        if (auto *aliasCtx = context->identifier()) {
            req.alias = aliasCtx->getText();
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
    std::any ASTGenVisitor::visitPrim(ANTLRParser::PrimContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitIdentifier(ANTLRParser::IdentifierContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitLit_string(ANTLRParser::Lit_stringContext *context) DEFAULT_VISIT

    std::any ASTGenVisitor::visitGlobal(ANTLRParser::GlobalContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitDeclaration(ANTLRParser::DeclarationContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitDecl_data(ANTLRParser::Decl_dataContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitDecl_function(ANTLRParser::Decl_functionContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitFunction_argument(ANTLRParser::Function_argumentContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitArgument_state(ANTLRParser::Argument_stateContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitData_storage(ANTLRParser::Data_storageContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_export(ANTLRParser::Stat_exportContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExport_discrete(ANTLRParser::Export_discreteContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitLit_char(ANTLRParser::Lit_charContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitLit_number(ANTLRParser::Lit_numberContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitLit_boolean(ANTLRParser::Lit_booleanContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitGeneric_block(ANTLRParser::Generic_blockContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitGeneric_content(ANTLRParser::Generic_contentContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitRoot_statement(ANTLRParser::Root_statementContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_silly(ANTLRParser::Stat_sillyContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitData_item(ANTLRParser::Data_itemContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitData_details(ANTLRParser::Data_detailsContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitFunction_details(ANTLRParser::Function_detailsContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_bind(ANTLRParser::Stat_bindContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitBind_block(ANTLRParser::Bind_blockContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitBind_item(ANTLRParser::Bind_itemContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_return(ANTLRParser::Stat_returnContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprGrouped(ANTLRParser::ExprGroupedContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprMember(ANTLRParser::ExprMemberContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprCall(ANTLRParser::ExprCallContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprPostfix(ANTLRParser::ExprPostfixContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprPrefix(ANTLRParser::ExprPrefixContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprPriorityHigh(ANTLRParser::ExprPriorityHighContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprMultiplicative(ANTLRParser::ExprMultiplicativeContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprAdditive(ANTLRParser::ExprAdditiveContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprPriorityLow(ANTLRParser::ExprPriorityLowContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprComparison(ANTLRParser::ExprComparisonContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprLogical(ANTLRParser::ExprLogicalContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprAssign(ANTLRParser::ExprAssignContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprPrimary(ANTLRParser::ExprPrimaryContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExpr_list(ANTLRParser::Expr_listContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_postfix(ANTLRParser::Op_postfixContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_prefix(ANTLRParser::Op_prefixContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_priority_high(ANTLRParser::Op_priority_highContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_multiplicative(ANTLRParser::Op_multiplicativeContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_additive(ANTLRParser::Op_additiveContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_priority_low(ANTLRParser::Op_priority_lowContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_comparison(ANTLRParser::Op_comparisonContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_logical(ANTLRParser::Op_logicalContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitOp_assign(ANTLRParser::Op_assignContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitLit(ANTLRParser::LitContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_end(ANTLRParser::Stat_endContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_skip(ANTLRParser::Stat_skipContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_if(ANTLRParser::Stat_ifContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_iteration(ANTLRParser::Stat_iterationContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitExprIndex(ANTLRParser::ExprIndexContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitType_spec(ANTLRParser::Type_specContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitLit_array(ANTLRParser::Lit_arrayContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_switch(ANTLRParser::Stat_switchContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitSwitch_arm(ANTLRParser::Switch_armContext *context) DEFAULT_VISIT
    std::any ASTGenVisitor::visitStat_fall(ANTLRParser::Stat_fallContext *context) DEFAULT_VISIT
}
