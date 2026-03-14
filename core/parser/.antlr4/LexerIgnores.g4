/**
 *
 * This is the ANTLR grammar file for the Juggernyaut Lexer!
 *
 * (Comments and whitespace!)
 *
**/

lexer grammar LexerIgnores;

// Options
options {
    language=Cpp;
}

import LexerFragments;

// Whitespace
CHARS_IGNORE_LIST
    : WHITESPACE_
        -> skip
    ;

// Comments
COMMENT_BLOCK
    : '/*' .*? '*/'
        -> skip
    ; /* Multilinear comments are never processed for code generation */
COMMENT_LINE
    : '//' ~[\n]*
        -> skip
    ; /* Comments end when a new line starts! */
