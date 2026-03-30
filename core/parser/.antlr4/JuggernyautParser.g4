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
    : (expr SYM_SEMICOLON | SYM_SEMICOLON)*
        EOF
    ; /* This is the initial scope in a file! */

// Expressions
expr
    : stat_import
    | TMP
    | TMP2
    ;

// Statements
stat_import
    : KWD_IMPORT LIT_STRING
    ;
