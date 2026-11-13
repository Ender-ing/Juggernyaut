/**
 *
 * This is the ANTLR grammar file for Juggernyaut!
 * All syntax and parser rules are defined here.
 *
 * (Only contains finalised implementations!)
 *
 * (ANTLR4: https://github.com/antlr/antlr4/blob/master/doc/lexicon.md#keywords)
 *
**/

parser grammar JuggernyautParser;

// Manage options
options {
    tokenVocab=JuggernyautLexer;
    language=Cpp;
}

//// Parser Rules

prog
    : global* EOF
    ; /* This is the initial scope in a file! */

// Globals
global
    : root_statement
    | KWD_EXPORT? declaration
    ;

// Zoned
generic_block
    : SYM_CURLY_BRACKET_LEFT generic_content* SYM_CURLY_BRACKET_RIGHT
    ;
generic_content
    : statement
    | declaration
    | stat_return
    | stat_end
    | stat_skip
    | stat_if
    | stat_iteration
    | generic_block // Allow nested scopes!
    ;

// Statements
statement
    : stat_empty // Just avoiding annoying bugs
    | stat_expr
    | stat_silly
    ;
root_statement
    : stat_empty // Just avoiding annoying bugs
    | stat_import
    | stat_export
    | stat_bind
    | stat_silly
    ;

// Declarations
declaration
    : decl_data
    | decl_function
    ;

// Import statement
stat_import
    : import_discrete
    | import_encapsulate
    ;
import_discrete
    : KWD_IMPORT
        SYM_CURLY_BRACKET_LEFT import_list? SYM_CURLY_BRACKET_RIGHT
        KWD_FROM import_source SYM_SEMICOLON
    ;
import_encapsulate
    : KWD_IMPORT import_source KWD_AS identifier SYM_SEMICOLON
    ;
import_list
    : identifier (SYM_COMMA identifier)*
    ;
import_source
    : import_library
    | import_path
    ;
import_path
    : LIT_STRING
    ;
import_library
    : identifier (SYM_DOT identifier)?
    ;

// Export statement
stat_export
    : export_discrete
    ;
export_discrete
    : KWD_EXPORT
        identifier (SYM_COMMA identifier)*
        SYM_SEMICOLON
    ;

// Bind statement
stat_bind
    : KWD_EXPORT? KWD_BIND (bindMode=lit_string?) (bind_item | bind_block)
    ;
bind_block
    : SYM_CURLY_BRACKET_LEFT (bind_item*) SYM_CURLY_BRACKET_RIGHT
    ;
bind_item
    : (function_details | (data_storage data_details))
        (KWD_FROM originalName=lit_string)?
        SYM_SEMICOLON
    ;

// Functional/loop statements
stat_return
    : KWD_RETURN (value=expr?) SYM_SEMICOLON
    ;
stat_end
    : KWD_END SYM_SEMICOLON
    ;
stat_skip
    : KWD_SKIP SYM_SEMICOLON
    ;

// Logical statements
stat_if
    : KWD_IF SYM_PARENTHESES_LEFT condition=expr SYM_PARENTHESES_RIGHT body=generic_block
        (KWD_ELSE (elseBody=generic_block | chainedFlow=stat_if))?
    ;
stat_iteration
    : KWD_WHILE SYM_PARENTHESES_LEFT condition=expr SYM_PARENTHESES_RIGHT body=generic_block
    | KWD_DO body=generic_block
        KWD_WHILE SYM_PARENTHESES_LEFT condition=expr SYM_PARENTHESES_RIGHT SYM_SEMICOLON
    ;

// Expression statement
stat_expr
    : expr SYM_SEMICOLON
    ;

// Empty statement
stat_empty
    : SYM_SEMICOLON
    ;

// Silly statements
stat_silly
    : CAT SYM_SEMICOLON
    | KITTY SYM_SEMICOLON
    | MEOW SYM_SEMICOLON
    | BAD_CODE SYM_SEMICOLON
    ;

// Data declaration
decl_data
    : data_storage data_item (SYM_COMMA data_item)* SYM_SEMICOLON
    ;
data_item
    : details=data_details (SYM_EQUAL value=expr)?
    ;
data_details
    :  name=identifier (SYM_COLON type=identifier)
    ;
data_storage
    : KWD_LET
    | KWD_CONST
    | KWD_REF
    ;
// Function declaration
decl_function
    : details=function_details body=generic_block
    ;
function_details
    : KWD_FUNC name=identifier
        SYM_PARENTHESES_LEFT
            (function_argument (SYM_COMMA function_argument)*)?
        SYM_PARENTHESES_RIGHT
        (SYM_ARROW_RIGHT returnType=identifier)?
    ;
function_argument
    : state=argument_state? details=data_details (SYM_EQUAL defaultValue=expr)?
    ;
argument_state
    : KWD_CONST
    | KWD_REF
    ;

// NOTE: from here on, no semicolons are allowed within rules

// Expressions (Ordered from HIGHEST precedence at top to LOWEST at bottom)
expr
    : SYM_PARENTHESES_LEFT expr SYM_PARENTHESES_RIGHT                   # ExprGrouped
    | target=expr SYM_DOT member=identifier                             # ExprMember
    | callee=expr SYM_PARENTHESES_LEFT expr_list? SYM_PARENTHESES_RIGHT # ExprCall
    | operand=expr op=op_postfix                                        # ExprPostfix
    | <assoc=right> op=op_prefix operand=expr                           # ExprPrefix
    | <assoc=right> left=expr op=op_priority_high right=expr            # ExprPriorityHigh
    | left=expr op=op_multiplicative right=expr                         # ExprMultiplicative
    | left=expr op=op_additive right=expr                               # ExprAdditive
    | left=expr op=op_priority_low right=expr                           # ExprPriorityLow
    | left=expr op=op_comparison right=expr                             # ExprComparison
    | left=expr op=op_logical right=expr                                # ExprLogical
    | <assoc=right> left=expr op=op_assign right=expr                   # ExprAssign
    | prim                                                              # ExprPrimary
    ;

expr_list
    : expr (SYM_COMMA expr)*
    ;

// Operations

op_postfix
    : SYM_PLUS_PLUS       // ++
    | SYM_MINUS_MINUS     // --
    ;
op_prefix
    : SYM_PLUS_PLUS       // ++
    | SYM_MINUS_MINUS     // --
    | SYM_PLUS            // +
    | SYM_MINUS           // -
    | SYM_NOT             // !
    | SYM_TILDE           // ~
    ;
op_priority_high
    : SYM_STAR_STAR       // **
    | SYM_TILDE_TILDE     // ~~
    ;
op_multiplicative
    : SYM_STAR                  // *
    | SYM_SLASH                 // /
    | SYM_BACKSLASH             // \
    | SYM_BACKSLASH_BACKSLASH   // \\
    | SYM_PERCENT               // %
    | SYM_PERCENT_PERCENT       // %%
    | SYM_AT                    // @
    | SYM_AT_AT                 // @@
    ;
op_additive
    : SYM_PLUS                  // +
    | SYM_MINUS                 // -
    | SYM_AND                   // &
    | SYM_OR                    // |
    | SYM_CARET                 // ^
    | SYM_SHIFT_LEFT            // <<
    | SYM_SHIFT_RIGHT           // >>
    | SYM_HASH                  // #
    | SYM_HASH_HASH             // ##
    ;
op_priority_low
    : SYM_DOT_DOT         // ..
    | SYM_TILDE           // ~
    ;
op_comparison
    : SYM_EQUAL_EQUAL     // ==
    | SYM_NOT_EQUAL       // !=
    | SYM_LESS            // <
    | SYM_LESS_EQUAL      // <=
    | SYM_GREATER         // >
    | SYM_GREATER_EQUAL   // >=
    ;
op_logical
    : SYM_AND_AND         // &&
    | SYM_OR_OR           // ||
    | SYM_CARET_CARET     // ^^
    ;
op_assign
    : SYM_EQUAL                         // =
    | SYM_PLUS_EQUAL                    // +=
    | SYM_MINUS_EQUAL                   // -=
    | SYM_STAR_EQUAL                    // *=
    | SYM_STAR_STAR_EQUAL               // **=
    | SYM_SLASH_EQUAL                   // /=
    | SYM_BACKSLASH_EQUAL               // \=
    | SYM_BACKSLASH_BACKSLASH_EQUAL     // \\=
    | SYM_PERCENT_EQUAL                 // %=
    | SYM_PERCENT_PERCENT_EQUAL         // %%=
    | SYM_AND_EQUAL                     // &=
    | SYM_OR_EQUAL                      // |=
    | SYM_CARET_EQUAL                   // ^=
    | SYM_SHIFT_LEFT_EQUAL              // <<=
    | SYM_SHIFT_RIGHT_EQUAL             // >>=
    | SYM_AT_EQUAL                      // @=
    | SYM_AT_AT_EQUAL                   // @@=
    | SYM_HASH_EQUAL                    // #=
    | SYM_HASH_HASH_EQUAL               // ##=
    | SYM_TILDE_EQUAL                   // ~=
    | SYM_TILDE_TILDE_EQUAL             // ~~=
    ;

// Primary data
prim
    : identifier
    | lit
    ;

// Identifiers
identifier
    : IDENTIFIER
    ;

// Literals
lit
    : lit_boolean
    | lit_number
    | lit_char
    | lit_string
    ;
lit_boolean
    : LIT_TRUE
    | LIT_FALSE
    ;
lit_number
    : LIT_FLOAT
    | LIT_INT
    | LIT_NUM_REP
    ;
lit_char
    : LIT_CHAR
    ;
lit_string
    : LIT_STRING
    ;
