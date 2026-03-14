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

program
    : SYM_SEMICOLON*
        (expr SYM_SEMICOLON+)*
        EOF
    ; /* This is the initial scope in a file! */

// Expressions
expr
    : TMP
    | TMP2
    ;
