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

SYM_DOT
    : '.'
    ;

SYM_CURLY_BRACKET_LEFT
    : '{'
    ;
SYM_CURLY_BRACKET_RIGHT
    : '}'
    ;

/*SYM_COLON
    : ':'
    ;*/
