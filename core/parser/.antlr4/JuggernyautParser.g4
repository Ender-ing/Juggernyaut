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
    : statement* EOF
    ; /* This is the initial scope in a file! */

// Statements
statement
    : stat_empty
    | stat_expr
    | stat_import
    | TMP SYM_SEMICOLON
    | TMP2 SYM_SEMICOLON
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
    : KWD_IMPORT import_source KWD_AS IDENTIFIER
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
    : IDENTIFIER (SYM_DOT IDENTIFIER)?
    ;

// Expression statement
stat_expr
    : expr SYM_SEMICOLON
    ;

// Empty statement
stat_empty
    : SYM_SEMICOLON
    ;

// NOTE: from here on, no semicolons are allowed within rules

// Expressions
expr
    : /*expr SYM_DOT IDENTIFIER  // Member call
    |*/ prim
    ;

// Primary data
prim
    : lit_string
    ;

// Identifiers
identifier
    : IDENTIFIER
    | TYPE_IDENTIFIER
    ;

// Literals
lit_string
    : LIT_STRING
    ;
