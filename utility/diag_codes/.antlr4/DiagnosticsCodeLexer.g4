/**
 *
 * (ANTLR4: https://github.com/antlr/antlr4/blob/master/doc/lexicon.md#keywords)
 *
**/

lexer grammar DiagnosticsCodeLexer;

// Manage options
options {
    language=Cpp;
}

//// Lexer tokens

fragment WHITESPACE_
    : [ \t\r\n]
    ;
fragment DIGIT_
    : [0-9]
    ;
fragment IDENTIFIER_CHARS_
    : [A-Z0-9_]
    ;
fragment IDENTIFIER_CHARS_START_
    : [A-Z_]
    ;
fragment VAR_REF_
    : '$'
    ;
fragment VAR_NAME_
    : VAR_REF_ IDENTIFIER_CHARS_START_ IDENTIFIER_CHARS_*
    ;

// Whitespace
CHARS_IGNORE_LIST
    : WHITESPACE_
        -> skip
    ;

SYM_BRACKET_LEFT
    : '['
    ;
SYM_BRACKET_RIGHT
    : ']'
    ;
SYM_COLON
    : ':'
    ;
SYM_COMMA
    : ','
    ;

ID
    : DIGIT_+
    ;


TYP_NUMBER
    : 'NUM'
    ;
TYP_TEXT
    : 'TXT'
    ;

VAR
    : VAR_NAME_
    ;

LINE_OPEN
    : '-'
        -> pushMode(INLINE)
    ;

CONTENT
    : ~('\r' | '\n' | '\\' | '$')
    ;

mode INLINE;

LINE_CLOSE
    : ('\r\n' | '\n')
        -> popMode
    ;
LINE_CONTENT
    : ~('\r' | '\n' | '$' | '\\')+
    ;
LINE_SLASH_ESCAPE
    : '\\' '\\'
    ;
LINE_CALL_ESCAPE
    : '\\' VAR_REF_
    ;
LINE_CALL
    : VAR_NAME_
    ;
LINE_IGNORE_LIST
    : '\r'
        -> skip
    ;
