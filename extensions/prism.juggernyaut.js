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

    // Keywords
    'keyword': /\b(?:TMP|TMP2|import|as|from)\b/,

    // Literals
    'string': {
        pattern: /(["])(?:\\(?:\r\n|[\s\S])|(?!\1)[^\\\r\n])*(\1|\n)/, // TO-DO: change ["] to ["'] once chars are implemented
        greedy: true
    }
};
Prism.languages.jug = Prism.languages.juggernyaut;
