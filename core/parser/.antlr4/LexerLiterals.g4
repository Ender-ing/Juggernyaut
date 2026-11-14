/**
 *
 * This is the ANTLR grammar file for the Juggernyaut Lexer!
 *
 * (Literals)
 *
**/

lexer grammar LexerLiterals;

// Options
options {
    language=Cpp;
}

import LexerFragments;

// Booleans
LIT_TRUE
    : 'true'
    ;
LIT_FALSE
    : 'false'
    ;

// Numbers
// (extra leading zeros are allowed for normal numbers)

// Floating-point numbers: 9.4, 1.0, 0.5, 1.5e-3
LIT_FLOAT
    : ([0-9]*) '.' [0-9]+ ([eE] [+-]? [0-9]+)?
    ;
// Integers: decimal (0, 1, -1), standard prefixes (0b, 0o, 0x), and radix (1d..36d)
LIT_INT
    : [0-9] [0-9]*                                    // Decimal:     1, 486
    ;
LIT_NUM_REP // (NO EXTRA LEADING ZEROS ARE ALLOWED HERE!)
    : '0' [bB] [01]+                                  // Binary:      0b1010
    | '0' [oO] [0-7]+                                 // Octal:       0o10
    | '0' [xX] [0-9a-fA-F]+                           // Hexadecimal: 0xFF, 0xff
    | ([1-9] | [12] [0-9] | '3' [0-6]) 'd' [0-9a-zA-Z]+ // Radix 1..36: 1d111 .. 36dZ
    ;
// Catch malformed numbers (001, 0d111, 37dZ, 123abc) as a single error token
// instead of splitting them into a valid number + identifier/extra numbers
INVALID_LIT_NUMBER_REP
    : '0' [0-9]+ [0-9a-zA-Z_.]*                       // Leading zeros: 001, 00
    | '0' 'd' [0-9a-zA-Z_]*                           // Base 0:        0d...
    | ('3' [7-9] | [4-9] [0-9] | [1-9] [0-9] [0-9]+) 'd' [0-9a-zA-Z_]* // Base 37+: 37dZ
    | [0-9]+ ('.' [0-9]+)? [a-zA-Z_]+ [0-9a-zA-Z_]*   // Trailing junk: 123abc, 0xGG
    ;

// Chars

LIT_CHAR
    : '\'' LONE_CHAR_ '\''
    ;
// Catch empty (''), multi-char ('ab'), or unclosed ('a) char literals
INVALID_1_LIT_CHAR
    : '\'\''
    | '\'' LONE_CHAR_+ '\''
    | '\'' LONE_CHAR_* [\r\n]
    ;
INVALID_2_LIT_CHAR
    : '\'' ('\\' NEG_ESCAPES_ | LONE_CHAR_)* ([\r\n] | '\'')
    ; /* Chars containing invalid escape characters */


// Strings

LIT_STRING
    : '"' CHAIN_CHAR_* '"'
    ;
INVALID_1_LIT_STRING
    : '"' CHAIN_CHAR_* [\r\n]
    ; /* Unclosed strings */
INVALID_2_LIT_STRING
    : '"' ('\\' NEG_ESCAPES_ | CHAIN_CHAR_)* ([\r\n] | '"')
    ; /* Strings containing invalid escape characters */
