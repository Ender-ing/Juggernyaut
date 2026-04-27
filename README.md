# Juggernyaut

An experimental general-purpose programming language in the making!

> [!NOTE]
> On Windows, Developer Mode must be enabled to allow symbolic linking without the need for elevated user privileges!

## Overview

- `/compiler`:
  The CLI wrapper and the bridge between the front-end and the back-end.
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
- `/tests`:
  Test files.
- `/utility`:
  - `common`:
    The common library
  - `diag_codes`:
    Diagnostic code generator

## Memory debugging

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./NAME
```

## Pull Requests

As long as a pull request follows the [style guide](https://ender.ing/docs/category/style-guides),
and it doesn't mess with the structure of the repository, it'd be gladly accepted!

> [!WARNING]
> If a contribution requires the addition of a new dependency or resource, it won't be accepted
> unless ` -SEC` is appended to the name of the pull request - *which is a trigger for security scans*!

## Diagnostic codes

- Temporary placeholder: 1
- Lexer: 1xxxxx
- Parser: 2xxxxx
