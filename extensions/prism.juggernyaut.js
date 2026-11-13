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
    'keyword': /\b(?:cat|kitty|meow|bad_code|import|export|bind|let|ref|const|as|from|return|if|else|while|do|end|skip)\b/,

    // Literals
    'string': {
        pattern: /(["])(?:\\(?:\r\n|[\s\S])|(?!\1)[^\\\r\n])*(\1|\n)/, // TO-DO: change ["] to ["'] once chars are implemented
        greedy: true
    }
};
Prism.languages.jug = Prism.languages.juggernyaut;
