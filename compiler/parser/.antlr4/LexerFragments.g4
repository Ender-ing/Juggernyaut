/**
 *
 * This is the ANTLR grammar file for the Juggernyaut Lexer!
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
