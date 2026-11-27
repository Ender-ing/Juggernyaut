const { CHAIN_CHAR_, LONE_CHAR_ } = require('./fragments');
const SYM = require('./symbols');

const LIT_TRUE = 'true';
const LIT_FALSE = 'false';

module.exports = {
    LIT_TRUE,
    LIT_FALSE,

    rules: {
        _lit: $ => choice(
            $.lit_boolean,
            $.lit_number,
            $.lit_char,
            $.lit_string,
            $.lit_array
        ),

        // Booleans
        lit_boolean: _ => choice(LIT_TRUE, LIT_FALSE),

        // Numbers
        lit_number: $ => choice(
            $.lit_float,
            $.lit_int,
            $.lit_num_rep
        ),

        // Floating-point numbers: 9.4, 1.0, 0.5, 1.5e-3
        lit_float: _ => token(choice(
            /[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?/,
            /[0-9]+\.([eE][+-]?[0-9]+)?/
        )),

        // Decimal integers: 0, 1, 486
        lit_int: _ => token(/[0-9]+/),

        // Binary (0b), Octal (0o), Hex (0x), and Radix 1..36 (1d..36d)
        lit_num_rep: _ => token(choice(
            /0[bB][01]*/,
            /0[oO][0-7]*/,
            /0[xX][0-9a-fA-F]*/,
            /([1-9]|[12][0-9]|3[0-6])d[0-9a-zA-Z]*/
        )),

        // Chars
        lit_char: _ => token(seq('\'', LONE_CHAR_, '\'')),

        // Strings
        lit_string: _ => token(seq('"', repeat(CHAIN_CHAR_), '"')),

        // Array literal
        lit_array: $ => seq(
            SYM.SYM_SQUARE_BRACKET_LEFT,
            optional(field('elements', $.expr_list)),
            SYM.SYM_SQUARE_BRACKET_RIGHT
        )
    }
};
