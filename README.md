# Juggernyaut

An experimental general-purpose programming language in the making!

> [!NOTE]
> On Windows, Developer Mode must be enabled to allow symbolic linking without the need for elevated user privileges!

## Overview

- `/compiler`:
Includes the CLI wrapper.

- `/core`:
Includes the core components of the compilation process! (e.g. lexter, parser, etc.)

- `/extensions`:
Includes extensions for well-known code editors.

- `/server`:
Includes the server language. (Follows the LSP)

## Memory debugging

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./NAME
```

## Releases

The comment of the last commit made before a release must follow the pattern the `* -FULL`.
This is done as to ensure all architectures are built and tested successfully!
