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

// Strings & Chars
fragment ESCAPES_
    : [nrt0"'bfv\\]
    ; /* Valid escape characters after '\': \n, \r, \t, \0, \", \', \b, \f, \v, \\ */
fragment NEG_ESCAPES_
    : ~[nrt0"'bfv\\\r\n]
    ; /* Invalid escape characters after '\' */
fragment IMP_CHAR_
    : '\\' ESCAPES_
    | '\\' '\r'? '\n' // line continuation escape
    ;
fragment CHAIN_CHAR_
    : ~["\\\r\n]
    | IMP_CHAR_
    ;
fragment LONE_CHAR_
    : ~['\\\r\n]
    | IMP_CHAR_
    ;

fragment STANDARD_IDENTIFIER_CHARS
    : [a-zA-Z0-9_]
    ; /* All supported identifier name characters */
