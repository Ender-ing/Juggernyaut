/**
 *
 * This is the ANTLR grammar file for the Juggernyaut Lexer!
 *
 * (Literals)
 *
**/

lexer grammar LexerLiterals;

// Options
options {
    language=Cpp;
}

import LexerFragments;

LIT_STRING
    : '"' CHAIN_CHAR_* '"'
    ;
INVALID_1_LIT_STRING
    : '"' CHAIN_CHAR_* [\r\n]
    ; /* Unclosed strings */
INVALID_2_LIT_STRING
    : '"' ('\\' NEG_ESCAPES_ | CHAIN_CHAR_)* ([\r\n] | '"')
    ; /* Strings containing invalid escape characters */
