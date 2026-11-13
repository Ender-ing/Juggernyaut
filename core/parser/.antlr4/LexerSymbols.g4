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

SYM_EQUAL
    : '='
    ;

SYM_DOT
    : '.'
    ;
SYM_COMMA
    : ','
    ;

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

SYM_ARROW_RIGHT
    : '->'
    ;
