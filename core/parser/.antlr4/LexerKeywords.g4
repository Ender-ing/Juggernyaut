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

// Symbol relations
KWD_IMPORT
    : 'import'
    ;
KWD_EXPORT
    : 'export'
    ;
KWD_BIND
    : 'bind'
    ;

// Declarations
KWD_LET
    : 'let'
    ;
KWD_REF
    : 'ref'
    ;
KWD_CONST
    : 'const'
    ;
KWD_FUNC
    : 'func'
    ;

// Flow
KWD_FROM
    : 'from'
    ;
KWD_AS
    : 'as'
    ;

// Silly
CAT
    : 'cat'
    ;
KITTY
    : 'kitty'
    ;
MEOW
    : 'meow'
    ;
BAD_CODE
    : 'bad_code'
    ;
