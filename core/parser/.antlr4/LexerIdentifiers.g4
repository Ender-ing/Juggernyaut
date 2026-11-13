/**
 *
 * This is the ANTLR grammar file for the Juggernyaut Lexer!
 *
 * (Comments and whitespace!)
 *
**/

lexer grammar LexerIdentifiers;

// Options
options {
    language=Cpp;
}

import LexerFragments;

IDENTIFIER
    : [a-zA-Z_] (STANDARD_IDENTIFIER_CHARS)*
    ; // general identifiers
