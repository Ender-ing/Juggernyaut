/**
 *
 * (ANTLR4: https://github.com/antlr/antlr4/blob/master/doc/lexicon.md#keywords)
 *
**/

parser grammar DiagnosticsCodeParser;

// Manage options
options {
    tokenVocab=DiagnosticsCodeLexer;
    language=Cpp;
}

//// Parser Rules

list
    : code*
        EOF
    ; /* This is the initial scope in a file! */

code
    : ID function
    | ID static
    ;

function
    : SYM_BRACKET_LEFT (variable_def*) SYM_BRACKET_RIGHT SYM_COLON line+
    ;

static
    : SYM_COLON line+
    ;

line
    : LINE_OPEN content* LINE_CLOSE
    ;

content
    : LINE_CONTENT
    | escape
    | variable_call
    ;
escape
    : LINE_CALL_ESCAPE
    | LINE_SLASH_ESCAPE
    ;

variable_call
    : LINE_CALL
    ;

// Functions

variable_type
    : TYP_NUMBER
    | TYP_TEXT
    ;

variable_def
    : variable_type VAR
    ;

