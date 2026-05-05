# Juggernyaut

An experimental general-purpose programming language with *no particular purpose* in the making!

> [!NOTE]
> On Windows, Developer Mode must be enabled to allow symbolic linking without the need for elevated user privileges!

## Plans

- [ ] Core (Front-End)
  > Goal: To provide a pruned and valid source tree.
  - [ ] Lexer & Parser
    > Diagnostics: syntax diagnostics, import diagnostics.
    >
    > Goal: To attach a valid AST to all sources.
    - ...
  - [ ] Semantic Analyser
    > Diagnostics: Cross-Source type-checking.
    >
    > Goal: To attach a valid symbol table to all sources and validate the AST.
    - ...
  - [ ] Integrity & Reachability Checker
    > Diagnostics: Entry fuunction diagnostics, and build-predictive diagnostics.
    >
    > Goal: To designate one entry source as the exclusive entry point,
    > and attach a pruned AST and a pruned symbols table to each source.
    - ...
- [ ] Configs
  > Diagnostics: Misconfigurations
  >
  > Goal: To modify Sessions in a uniform way across different front-end callers.
  - ...
- [ ] Package Manager
  > Diagnostics: -
  >
  > Goal: -
  - ...
- [ ] Compiler (Includes Back-End)
  > Goal: To generate a valid binary.
  - [ ] LLVM IR Generator
    > Diagnostics: -
    >
    > Goal: To attach valid LLVM IR to each source.
    - ...
  - [ ] LLVM workflow
    > Diagnostics: -
    >
    > Goal: To verify, optimise, and generate an object file from each source.
    - ...
  - [ ] LLD workflow
    > Diagnostics: -
    >
    > Goal: To generate a binary from object files.
    - ...
- [ ] Server
  > Goal: To provide front-end diagnostics, and intellisense.
  - ...
- [ ] Extensions
  > Goal: Enable easy use of the server and compiler in IDEs.
  - ...

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
- Configs: 6xxxxx
