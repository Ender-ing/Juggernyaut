# Juggernyaut

An experimental general-purpose programming language in the making!

> [!NOTE]
> On Windows, Developer Mode must be enabled to allow symbolic linking without the need for elevated user privileges!

## Overview

- `/compiler`:
  The CLI wrapper.
- `/configs`:
  Processing `jug.toml` config files
- `/core`:
  Includes the core components of the compilation process! (e.g. lexer, parser, etc.)  
  - `data`:
    Data structures that are meant to be shared between different stages.
  - `diagnostics`:
    Holds the shared `<Diagnostic>` and functions that generate it.
  - `parser`:
    Generates a parser AST and links processing contexts.
  - `session`
    Kick starts the entire workflow of the language's front-end!
- `/extensions`:
  Includes extensions for well-known code editors.
- `/server`:
  The language server for IDEs. (Follows the LSP)
- `/utility`:
  - `common`:
    The common library
  - `diag_codes`:
    Diagnostic code generator

## Memory debugging

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./NAME
```

## Releases

The comment of the last commit made before a release must follow the pattern `* -FULL`.
This is done as to ensure all architectures are built and tested successfully!

## Diagnostic codes

- Temporary placeholder: 1
- Lexer: 1xxxxx
- Parser: 2xxxxx
