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

// TMP
TMP
    : 'TMP'
    ;
TMP2
    : 'TMP2'
    ;

// Import
KWD_IMPORT
    : 'import'
    ;

// flow
KWD_FROM
    : 'from'
    ;
KWD_AS
    : 'as'
    ;
