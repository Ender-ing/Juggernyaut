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
    : [a-z_] (STANDARD_IDENTIFIER_CHARS)*
    ; // Function & data identifiers
TYPE_IDENTIFIER
    : [A-Z] (STANDARD_IDENTIFIER_CHARS)*
    ;
