// Auto-generated from extensions/tree-sitter-jug/grammar/base
Prism.languages.juggernyaut = {
    // Comments
    'comment': [
        {
            pattern: /\/\*([\s\S]*?)\*\//,
            greedy: true
        },
        {
            pattern: /\/\/.*/,
            greedy: true
        }
    ],

    // Literals
    'string': {
        pattern: /(["'])(?:\\(?:\r\n|[\s\S])|(?!\1)[^\\\r\n])*(\1|\n)/,
        greedy: true
    },
    'boolean': /\b(?:true|false)\b/,
    'number': /\b(?:0[bB][01]*|0[oO][0-7]*|0[xX][0-9a-fA-F]*|(?:[1-9]|[12][0-9]|3[0-6])d[0-9a-zA-Z]*|[0-9]*\.[0-9]+(?:[eE][+-]?[0-9]+)?||[0-9]+\.(?:[eE][+-]?[0-9]+)?|[0-9]+)\b/,

    // Keywords
    'keyword': /\b(?:import|export|bind|let|ref|const|func|type|abstract|from|as|return|if|else|while|for|do|end|skip|in|switch|case|fall|cat|kitty|meow|bad_code)\b/,

    // Operators & Punctuation
    'operator': /\*\*=|\\\\=|%%=|<<=|>>=|@@=|##=|~~=|->|\+\+|--|\*\*|\\\\|%%|\.\.|<<|>>|@@|##|~~|\+=|-=|\*=|/=|\\=|%=|&=|\|=|\^=|@=|#=|~=|==|!=|<=|>=|&&|\|\||\^\^|;|:|\.|,|\+|-|!|~|\*|/|\\|%|&|\||\^|@|#|=|<|>/,
    'punctuation': /[{}[\]();,.:]/
};
Prism.languages.jug = Prism.languages.juggernyaut;
