const SYM = require('../base/symbols');

module.exports = {
    rules: {
        op_postfix: _ => choice(
            SYM.SYM_PLUS_PLUS,
            SYM.SYM_MINUS_MINUS
        ),

        op_prefix: _ => choice(
            SYM.SYM_PLUS_PLUS,
            SYM.SYM_MINUS_MINUS,
            SYM.SYM_PLUS,
            SYM.SYM_MINUS,
            SYM.SYM_NOT,
            SYM.SYM_TILDE
        ),

        op_priority_high: _ => choice(
            SYM.SYM_STAR_STAR,
            SYM.SYM_TILDE_TILDE
        ),

        op_multiplicative: _ => choice(
            SYM.SYM_STAR,
            SYM.SYM_SLASH,
            SYM.SYM_BACKSLASH,
            SYM.SYM_BACKSLASH_BACKSLASH,
            SYM.SYM_PERCENT,
            SYM.SYM_PERCENT_PERCENT,
            SYM.SYM_AT,
            SYM.SYM_AT_AT
        ),

        op_additive: _ => choice(
            SYM.SYM_PLUS,
            SYM.SYM_MINUS,
            SYM.SYM_AND,
            SYM.SYM_OR,
            SYM.SYM_CARET,
            SYM.SYM_SHIFT_LEFT,
            SYM.SYM_SHIFT_RIGHT,
            SYM.SYM_HASH,
            SYM.SYM_HASH_HASH
        ),

        op_priority_low: _ => choice(
            SYM.SYM_DOT_DOT,
            SYM.SYM_TILDE
        ),

        op_comparison: _ => choice(
            SYM.SYM_EQUAL_EQUAL,
            SYM.SYM_NOT_EQUAL,
            SYM.SYM_LESS,
            SYM.SYM_LESS_EQUAL,
            SYM.SYM_GREATER,
            SYM.SYM_GREATER_EQUAL
        ),

        op_logical: _ => choice(
            SYM.SYM_AND_AND,
            SYM.SYM_OR_OR,
            SYM.SYM_CARET_CARET
        ),

        op_assign: _ => choice(
            SYM.SYM_EQUAL,
            SYM.SYM_PLUS_EQUAL,
            SYM.SYM_MINUS_EQUAL,
            SYM.SYM_STAR_EQUAL,
            SYM.SYM_STAR_STAR_EQUAL,
            SYM.SYM_SLASH_EQUAL,
            SYM.SYM_BACKSLASH_EQUAL,
            SYM.SYM_BACKSLASH_BACKSLASH_EQUAL,
            SYM.SYM_PERCENT_EQUAL,
            SYM.SYM_PERCENT_PERCENT_EQUAL,
            SYM.SYM_AND_EQUAL,
            SYM.SYM_OR_EQUAL,
            SYM.SYM_CARET_EQUAL,
            SYM.SYM_SHIFT_LEFT_EQUAL,
            SYM.SYM_SHIFT_RIGHT_EQUAL,
            SYM.SYM_AT_EQUAL,
            SYM.SYM_AT_AT_EQUAL,
            SYM.SYM_HASH_EQUAL,
            SYM.SYM_HASH_HASH_EQUAL,
            SYM.SYM_TILDE_EQUAL,
            SYM.SYM_TILDE_TILDE_EQUAL
        )
    }
};
