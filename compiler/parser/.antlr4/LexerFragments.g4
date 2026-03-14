/**
 *
 * This is the ANTLR grammar file for the PolarFrankie Lexer!
 *
 * (Fragments)
 *
**/

lexer grammar LexerFragments;

// Options
options {
    language=Cpp;
}

// Ignored whitespace
fragment WHITESPACE_
    : [ \t\r\n]
    ;
