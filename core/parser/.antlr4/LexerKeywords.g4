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
KWD_TYPE
    : 'type'
    ;
KWD_ABSTRACT
    : 'abstract'
    ;

// Flow
KWD_FROM
    : 'from'
    ;
KWD_AS
    : 'as'
    ;
KWD_RETURN
    : 'return'
    ;
KWD_IF
    : 'if'
    ;
KWD_ELSE
    : 'else'
    ;
KWD_WHILE
    : 'while'
    ;
KWD_FOR
    : 'for'
    ;
KWD_DO
    : 'do'
    ;
KWD_END
    : 'end'
    ;
KWD_SKIP
    : 'skip'
    ;
KWD_IN
    : 'in'
    ;
KWD_SWITCH
    : 'switch'
    ;
KWD_CASE
    : 'case'
    ;
KWD_FALL
    : 'fall'
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
