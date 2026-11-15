const { WHITESPACE_ } = require('./fragments');

module.exports = {
    extras: $ => [
        WHITESPACE_,
        $.comment_block,
        $.comment_line
    ],

    rules: {
        // Multilinear comments
        comment_block: _ => token(seq(
            '/*',
            /[^*]*\*+([^/*][^*]*\*+)*/,
            '/'
        )),

        // Single-line comments
        comment_line: _ => token(seq('//', /[^\n]*/))
    }
};
