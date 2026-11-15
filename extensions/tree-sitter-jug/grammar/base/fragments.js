// Ignored whitespace
const WHITESPACE_ = /[ \t\r\n]+/;

// Strings & Chars
// Valid escape characters after '\': \n, \r, \t, \0, \", \', \b, \f, \v, \\
const ESCAPES_ = /[nrt0"'bfv\\]/;
const NEG_ESCAPES_ = /[^nrt0"'bfv\\\r\n]/;

const IMP_CHAR_ = choice(
    seq('\\', ESCAPES_),
    /\\\r?\n/ // line continuation escape
);

const CHAIN_CHAR_ = choice(
    /[^"\\\r\n]/,
    IMP_CHAR_
);

const LONE_CHAR_ = choice(
    /[^'\\\r\n]/,
    IMP_CHAR_
);

const STANDARD_IDENTIFIER_CHARS = /[a-zA-Z0-9_]/;

module.exports = {
    WHITESPACE_,
    ESCAPES_,
    NEG_ESCAPES_,
    IMP_CHAR_,
    CHAIN_CHAR_,
    LONE_CHAR_,
    STANDARD_IDENTIFIER_CHARS
};
