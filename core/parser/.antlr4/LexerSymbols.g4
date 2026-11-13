/**
 *
 * This is the ANTLR grammar file for the Juggernyaut Lexer!
 *
 * (Symbols)
 *
**/

lexer grammar LexerSymbols;

// Options
options {
    language=Cpp;
}

SYM_SEMICOLON
    : ';'
    ;
SYM_COLON
    : ':'
    ;

SYM_DOT
    : '.'
    ;
SYM_COMMA
    : ','
    ;
SYM_ARROW_RIGHT
    : '->'
    ;

// Postfix & Prefix
SYM_PLUS_PLUS
    : '++'
    ;
SYM_MINUS_MINUS
    : '--'
    ;

// Prefix & Binary
SYM_PLUS
    : '+'
    ;
SYM_MINUS
    : '-'
    ;

// Prefix
SYM_NOT
    : '!'
    ;
SYM_TILDE
    : '~'
    ;

// Binary
SYM_STAR
    : '*'
    ;
SYM_STAR_STAR
    : '**'
    ;
SYM_SLASH
    : '/'
    ;
SYM_BACKSLASH
    : '\\'
    ;
SYM_BACKSLASH_BACKSLASH
    : '\\\\'
    ;
SYM_PERCENT
    : '%'
    ;
SYM_PERCENT_PERCENT
    : '%%'
    ;
SYM_DOT_DOT
    : '..'
    ;
SYM_AND
    : '&'
    ;
SYM_OR
    : '|'
    ;
SYM_SHIFT_LEFT
    : '<<'
    ;
SYM_SHIFT_RIGHT
    : '>>'
    ;
SYM_CARET
    : '^'
    ;
SYM_AT
    : '@'
    ;
SYM_AT_AT
    : '@@'
    ;
SYM_HASH
    : '#'
    ;
SYM_HASH_HASH
    : '##'
    ;
SYM_TILDE_TILDE
    : '~~'
    ;

// Assignment
SYM_EQUAL
    : '='
    ;
SYM_PLUS_EQUAL
    : '+='
    ;
SYM_MINUS_EQUAL
    : '-='
    ;
SYM_STAR_EQUAL
    : '*='
    ;
SYM_STAR_STAR_EQUAL
    : '**='
    ;
SYM_SLASH_EQUAL
    : '/='
    ;
SYM_BACKSLASH_EQUAL
    : '\\='
    ;
SYM_BACKSLASH_BACKSLASH_EQUAL
    : '\\\\='
    ;
SYM_PERCENT_EQUAL
    : '%='
    ;
SYM_PERCENT_PERCENT_EQUAL
    : '%%='
    ;
SYM_AND_EQUAL
    : '&='
    ;
SYM_OR_EQUAL
    : '|='
    ;
SYM_CARET_EQUAL
    : '^='
    ;
SYM_SHIFT_LEFT_EQUAL
    : '<<='
    ;
SYM_SHIFT_RIGHT_EQUAL
    : '>>='
    ;
SYM_AT_EQUAL
    : '@='
    ;
SYM_AT_AT_EQUAL
    : '@@='
    ;
SYM_HASH_EQUAL
    : '#='
    ;
SYM_HASH_HASH_EQUAL
    : '##='
    ;
SYM_TILDE_EQUAL
    : '~='
    ;
SYM_TILDE_TILDE_EQUAL
    : '~~='
    ;

// Comparison
SYM_EQUAL_EQUAL
    : '=='
    ;
SYM_NOT_EQUAL
    : '!='
    ;
SYM_LESS
    : '<'
    ;
SYM_LESS_EQUAL
    : '<='
    ;
SYM_GREATER
    : '>'
    ;
SYM_GREATER_EQUAL
    : '>='
    ;

// Logical
SYM_AND_AND
    : '&&'
    ;
SYM_OR_OR
    : '||'
    ;
SYM_CARET_CARET
    : '^^'
    ;

// Grouping
SYM_CURLY_BRACKET_LEFT
    : '{'
    ;
SYM_CURLY_BRACKET_RIGHT
    : '}'
    ;
SYM_PARENTHESES_LEFT
    : '('
    ;
SYM_PARENTHESES_RIGHT
    : ')'
    ;
