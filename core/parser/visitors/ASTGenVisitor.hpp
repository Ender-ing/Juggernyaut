/**
 * @brief
 * AST & ImportRequest generator visitor
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// ANTLR4 imports
#include "../antlr4.hpp"
#include "../antlr4ParserWithVisitor.hpp"

// Parser types
#include "../Parser.hpp"

namespace Parser::Visitors {
    using ANTLRParser = GeneratedParser::JuggernyautParser;

    class JUG_PARSER_API ASTGenVisitor final : public GeneratedParser::JuggernyautParserVisitor {
        private:
            const File *file = nullptr;
            const Diagnostics::ModifiableEvent &onDiagnostic;
            ParseResult &result;

            void emitDiagnostic(Diagnostics::Diagnostic &diag) ;
            ImportRequest extractImportSource(ANTLRParser::Import_sourceContext *context) ;

        public:
            ASTGenVisitor(
                const File *targetFile,
                const Diagnostics::ModifiableEvent &diagnosticCallback,
                ParseResult &parseResult
            ) : file(targetFile),
                onDiagnostic(diagnosticCallback),
                result(parseResult) {}

            ASTGenVisitor(const ASTGenVisitor&) = delete;
            ASTGenVisitor& operator=(const ASTGenVisitor&) = delete;

            std::any visitProg(ANTLRParser::ProgContext *context) override;
            std::any visitGlobal(ANTLRParser::GlobalContext *context) override;
            std::any visitStatement(ANTLRParser::StatementContext *context) override;
            std::any visitDeclaration(ANTLRParser::DeclarationContext *context) override;
            std::any visitStat_import(ANTLRParser::Stat_importContext *context) override;
            std::any visitImport_discrete(ANTLRParser::Import_discreteContext *context) override;
            std::any visitImport_encapsulate(ANTLRParser::Import_encapsulateContext *context) override;
            std::any visitImport_list(ANTLRParser::Import_listContext *context) override;
            std::any visitImport_source(ANTLRParser::Import_sourceContext *context) override;
            std::any visitImport_path(ANTLRParser::Import_pathContext *context) override;
            std::any visitImport_library(ANTLRParser::Import_libraryContext *context) override;
            std::any visitStat_expr(ANTLRParser::Stat_exprContext *context) override;
            std::any visitStat_empty(ANTLRParser::Stat_emptyContext *context) override;
            std::any visitDecl_data(ANTLRParser::Decl_dataContext *context) override;
            std::any visitDecl_function(ANTLRParser::Decl_functionContext *context) override;
            std::any visitFunction_argument(ANTLRParser::Function_argumentContext *context) override;
            std::any visitArgument_state(ANTLRParser::Argument_stateContext *context) override;
            std::any visitData_storage(ANTLRParser::Data_storageContext *context) override;
            std::any visitPrim(ANTLRParser::PrimContext *context) override;
            std::any visitIdentifier(ANTLRParser::IdentifierContext *context) override;
            std::any visitLit_string(ANTLRParser::Lit_stringContext *context) override;
            std::any visitStat_export(ANTLRParser::Stat_exportContext *context) override;
            std::any visitExport_discrete(ANTLRParser::Export_discreteContext *context) override;
            std::any visitLit_char(ANTLRParser::Lit_charContext *context) override;
            std::any visitLit_number(ANTLRParser::Lit_numberContext *context) override;
            std::any visitLit_boolean(ANTLRParser::Lit_booleanContext *context) override;
            std::any visitGeneric_block(ANTLRParser::Generic_blockContext *context) override;
            std::any visitGeneric_content(ANTLRParser::Generic_contentContext *context) override;
            std::any visitRoot_statement(ANTLRParser::Root_statementContext *context) override;
            std::any visitStat_silly(ANTLRParser::Stat_sillyContext *context) override;
            std::any visitData_item(ANTLRParser::Data_itemContext *context) override;
            std::any visitData_details(ANTLRParser::Data_detailsContext *context) override;
            std::any visitFunction_details(ANTLRParser::Function_detailsContext *context) override;
            std::any visitStat_bind(ANTLRParser::Stat_bindContext *context) override;
            std::any visitBind_block(ANTLRParser::Bind_blockContext *context) override;
            std::any visitBind_item(ANTLRParser::Bind_itemContext *context) override;
            std::any visitStat_return(ANTLRParser::Stat_returnContext *context) override;
            std::any visitExprGrouped(ANTLRParser::ExprGroupedContext *context) override;
            std::any visitExprMember(ANTLRParser::ExprMemberContext *context) override;
            std::any visitExprCall(ANTLRParser::ExprCallContext *context) override;
            std::any visitExprPostfix(ANTLRParser::ExprPostfixContext *context) override;
            std::any visitExprPrefix(ANTLRParser::ExprPrefixContext *context) override;
            std::any visitExprPriorityHigh(ANTLRParser::ExprPriorityHighContext *context) override;
            std::any visitExprMultiplicative(ANTLRParser::ExprMultiplicativeContext *context) override;
            std::any visitExprAdditive(ANTLRParser::ExprAdditiveContext *context) override;
            std::any visitExprPriorityLow(ANTLRParser::ExprPriorityLowContext *context) override;
            std::any visitExprComparison(ANTLRParser::ExprComparisonContext *context) override;
            std::any visitExprLogical(ANTLRParser::ExprLogicalContext *context) override;
            std::any visitExprAssign(ANTLRParser::ExprAssignContext *context) override;
            std::any visitExprPrimary(ANTLRParser::ExprPrimaryContext *context) override;
            std::any visitExpr_list(ANTLRParser::Expr_listContext *context) override;
            std::any visitOp_postfix(ANTLRParser::Op_postfixContext *context) override;
            std::any visitOp_prefix(ANTLRParser::Op_prefixContext *context) override;
            std::any visitOp_priority_high(ANTLRParser::Op_priority_highContext *context) override;
            std::any visitOp_multiplicative(ANTLRParser::Op_multiplicativeContext *context) override;
            std::any visitOp_additive(ANTLRParser::Op_additiveContext *context) override;
            std::any visitOp_priority_low(ANTLRParser::Op_priority_lowContext *context) override;
            std::any visitOp_comparison(ANTLRParser::Op_comparisonContext *context) override;
            std::any visitOp_logical(ANTLRParser::Op_logicalContext *context) override;
            std::any visitOp_assign(ANTLRParser::Op_assignContext *context) override;
            std::any visitLit(ANTLRParser::LitContext *context) override;
            std::any visitStat_end(ANTLRParser::Stat_endContext *context) override;
            std::any visitStat_skip(ANTLRParser::Stat_skipContext *context) override;
            std::any visitStat_if(ANTLRParser::Stat_ifContext *context) override;
            std::any visitStat_iteration(ANTLRParser::Stat_iterationContext *context) override;
            std::any visitExprIndex(ANTLRParser::ExprIndexContext *context) override;
            std::any visitType_spec(ANTLRParser::Type_specContext *context) override;
            std::any visitLit_array(ANTLRParser::Lit_arrayContext *context) override;
            std::any visitStat_switch(ANTLRParser::Stat_switchContext *context) override;
            std::any visitSwitch_arm(ANTLRParser::Switch_armContext *context) override;
            std::any visitStat_fall(ANTLRParser::Stat_fallContext *context) override;
    };
}
