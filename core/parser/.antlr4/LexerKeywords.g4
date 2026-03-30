/**
 *
 * This is the ANTLR grammar file for the Juggernyaut Lexer!
 *
 * (Keywords)
 *
**/

lexer grammar LexerKeywords;

// Options
options {
    language=Cpp;
}

KWD_IMPORT
    : 'import'
    ;
