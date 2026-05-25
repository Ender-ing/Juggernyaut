# Juggernyaut

An experimental general-purpose programming language with *no particular purpose* in the making!

> [!NOTE]
> On Windows, Developer Mode must be enabled to allow symbolic linking without the need for elevated user privileges!

## Overview

- `/compiler`:
  The CLI wrapper and the bridge between the front-end and the back-end.
- `/configs`: (*refactoring...*)
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
    Kick starts the entire workflow of the language's front-end within one project! (recommended for isolated language debugging)
  - `manager` (*refactoring...*)
    Kick starts concurrent sessions across multiple projects. (recommended for normal use)
- `/extensions`:
  Includes extensions for well-known code editors.
- `/package-manager`:" (*empty*)
  Includes the language libraries package manager
- `/server`:
  The language server for IDEs. (Follows the LSP)
- `/tests`:
  Test files.
- `/utility`:
  - `diag_codes`:
    Diagnostic code generator
  - `libraries`:
    Cross-project libraries

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

## Plans

- [ ] Core (Front-End)
  > Goal: To provide a pruned and valid source tree.
  - [ ] Lexer & Parser
    > Diagnostics: syntax diagnostics, import diagnostics.
    >
    > Goal: To attach an AST to all sources.
    - [ ] Literals
      - [ ] Boolean literals
      - [ ] Numerical literals
        - [ ] Decimal literals
        - [ ] Base literals
          - [ ] *base 1-36* literals (`XXd` - general format)
          - [ ] *Binary* literals (`0b` - base-2 alternative format)
          - [ ] *Octal* literals (`0o` - base-8 alternative format)
          - [ ] *Hexadecimal* literals (`0x` - base-16 alternative format)
      - [ ] Text literals
        - [ ] *Character* literals
        - [x] *String* literals
        - [ ] *String template* literals
      - ...
    - [ ] Value stores
      - [ ] Variables
      - [ ] Constants
      - [ ] Constant expressions (for compile-time use)
      - ...
    - [ ] Functions
      - [ ] basic functions
      - [ ] Compile-time functions
      - ...
    - [ ] `@` (*compile-time*) directives
      - ...
    - [ ] `import` statement
      - [ ] Source inclusion
        - [x] Local source inclusion
        - [ ] Library source inclusion
      - [ ] Symbol referencing
    - [ ] `export` statement
      - ...
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

## CMake

> [!IMPORTANT]
> You may use the SYSTEM keyword only when linking external dependencies.
>
> All warnings caused by dependencies made within this repository must be resolved within the source code itself!

## Releases

Releases can be triggered by creating a tag that starts with a `v`, and follows the versioning style:
`{major}.{minor}.{patch}-{release_name}.{release_patch}`

### Packaging

> [!NOTE]
> You need to [install the QT installer framework](https://download.qt.io/official_releases/qt-installer-framework/4.8.1/)
> on your machine in order to package the final toolchain!

Packaging is done through the cpack command.

