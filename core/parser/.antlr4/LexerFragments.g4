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
    : [\r\n"\\]
    ; /* Escape characters */
fragment NEG_ESCAPES_
    : ~[\r\n"\\]
    ; /* Escape characters */
fragment IMP_CHAR_
    : '\\' ESCAPES_
    | '\\\n' // newline escape
    | '\\\r\n' // newline escape
    ; /* Char member of a string */
fragment CHAIN_CHAR_
    : ~["\\\r\n]
    | IMP_CHAR_
    ; /* Char member of a string */
fragment LONE_CHAR_
    : ~['\\\r\n]
    | IMP_CHAR_
    ; /* Char member of a character */

fragment STANDARD_IDENTIFIER_CHARS
    : [a-zA-Z0-9_]
    ; /* All supported identifier name characters */
