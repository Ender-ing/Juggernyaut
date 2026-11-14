# Code Documentation Guide

This document defines the standards and conventions for documenting C++ header files (`.hpp`) across the **Juggernyaut** toolchain.

> [!IMPORTANT]
> **Only Document `.hpp` Header Files:** Doxygen API documentation belongs exclusively in `.hpp` header files, which define the public and internal API contracts. Implementation files (`.cpp`) are the implementation itself—not the API—and **must not** contain Doxygen documentation blocks (`///`, `@file`, `@brief`, etc.). Use regular `//` comments inside `.cpp` files only when explaining non-obvious implementation mechanics.

Our documentation pipeline uses **Doxygen** strictly as an XML AST/comment extractor (`GENERATE_XML = YES`, `GENERATE_HTML = NO`), **`@xpack/doxygen2docusaurus`** to transform that XML graph into cross-linked Markdown, and **Docusaurus v3** to render the final API reference (including live **Mermaid.js** diagrams and **KaTeX** math).

To generate the documentation locally into `.out/docs/api/`, run:

```bash
cmake --build .build --target jug-api-docs
```

> [!NOTE]
> **Explicit Briefs Are Required:** Auto-brief (`JAVADOC_AUTOBRIEF` and `QT_AUTOBRIEF`) is disabled in our Doxygen configuration. Every documented symbol **must** explicitly start its summary with `@brief`. Comments without `@brief` are treated strictly as detailed descriptions and will leave the symbol's summary blank in index tables.

---

## 1. In-File Documentation Conventions

### 1.1 Comment Style

In `.hpp` files, always use triple-slash (`///`) blocks above declarations, and trailing (`///<`) comments for inline fields or enum constants. Do not use `/* ... */` C-style doc blocks, and do not place Doxygen comments on definitions inside `.cpp` files.

```cpp
/// @brief Short one-paragraph summary of the symbol.
///
/// Detailed multi-paragraph explanation of the symbol's behavior, invariants,
/// thread-safety guarantees, and lifecycle.
```

* **`@brief` paragraph**: Starts immediately after `@brief` and ends at the first blank `///` line (or the next `@tag`). Because auto-brief is off, you can safely use periods, version numbers (e.g., `v1.0`), and abbreviations (`e.g.`, `i.e.`) inside `@brief` without cutting the summary short.
* **Detailed description**: Starts after the blank `///` line following `@brief`.

---

### 1.2 File Headers (`@file`)

Every header file (`.hpp`) must begin with a `@file` block attached to its folder group (see [Section 3](#3-grouping-components-by-folder)). Do not add `@file` blocks to `.cpp` implementation files:

```cpp
/// @file
/// @ingroup CoreDiagnostics
/// @brief Defines the core `Diagnostic` structure and severity levels.
///
/// This file provides the primary error-reporting primitives shared across
/// the lexer, parser, semantic analyzer, session, and driver stages.
```

---

### 1.3 Classes, Structs, Concepts, and Enums

Document the purpose, ownership semantics, and invariants of every user-facing or internal type:

```cpp
/// @ingroup CoreSession
/// @brief Manages the front-end compilation workflow for a single project.
///
/// A `Session` owns the source buffers, AST contexts, and diagnostic state
/// for an isolated compilation unit or project workspace.
///
/// @tparam AllocatorT Memory allocator strategy used for arena allocations.
///
/// @see Driver
/// @see Diagnostic
template <typename AllocatorT>
class Session {
    // ...
};
```

For `enum class` declarations and `struct` data members, use `///< @brief` for inline trailing documentation:

```cpp
/// @ingroup CoreDiagnostics
/// @brief Indicates the impact level of a reported #Diagnostic.
enum class DiagnosticSeverity {
    Hint,    ///< @brief Non-intrusive suggestion for code clarity or style.
    Warning, ///< @brief Potential issue that does not halt compilation.
    Error,   ///< @brief Fatal compilation error that invalidates the current stage.
};

/// @ingroup CoreData
/// @brief Represents a half-open byte range `[start, end)` within a source file.
struct SourceSpan {
    std::size_t start_offset = 0; ///< @brief Zero-based starting byte offset (inclusive).
    std::size_t end_offset   = 0; ///< @brief Zero-based ending byte offset (exclusive).
};
```

---

### 1.4 Functions and Methods

Every non-trivial function or method declaration in a `.hpp` file should document its template parameters, inputs/outputs, return values, preconditions/postconditions, and exceptions:

| Tag | Purpose |
| :--- | :--- |
| `@brief` | Mandatory concise summary of what the function does. |
| `@tparam Name` | Constraints and role of template parameter `Name`. |
| `@param[in] name` | Read-only input parameter. |
| `@param[out] name` | Output parameter populated by the function. |
| `@param[in,out] name` | Parameter read and mutated by the function. |
| `@return` | Description of the returned value. |
| `@retval value` | Meaning of a specific sentinel return value (e.g., `nullptr`, `false`). |
| `@throws Type` | Exception type thrown and the condition that triggers it. |
| `@pre` / `@post` | Preconditions expected on entry and postconditions guaranteed on exit. |

```cpp
/// @brief Parses a source file from disk and attaches its AST to the active session.
///
/// @param[in]     source_path Absolute path to the `.jug` file on disk.
/// @param[in,out] session     The #Session that will take ownership of the parsed AST.
/// @param[out]    out_diag    Receives syntax or I/O diagnostics emitted during parsing.
///
/// @return `true` if lexing and parsing succeeded without fatal errors; `false` otherwise.
/// @retval false If `source_path` does not exist or contains unrecoverable syntax errors.
///
/// @throws std::bad_alloc If the underlying AST arena fails to allocate memory.
/// @pre `source_path.is_absolute()` must be `true`.
/// @post If `true` is returned, `session.has_ast(source_path)` is guaranteed to be `true`.
///
/// @see Session
/// @see Diagnostic
[[nodiscard]] bool parse_source_file(
    const std::filesystem::path& source_path,
    Session& session,
    DiagnosticList& out_diag);
```

---

### 1.5 Member Sections Inside a Class (`@name`)

When a class has many methods (such as an ANTLR4 visitor or LSP endpoint), group related members under named subsections using `@name` and `/// @{ ... /// @}`:

```cpp
class ParserVisitor {
public:
    /// @name Literal Visitors
    /// @brief Methods that lower literal tokens into typed AST nodes.
    /// @{

    /// @brief Visits a numeric literal token (`0b`, `0o`, `0x`, or decimal).
    std::any visitNumericLiteral(NumericLiteralContext* ctx);

    /// @brief Visits a UTF-8 string or character literal token.
    std::any visitTextLiteral(TextLiteralContext* ctx);

    /// @}

    /// @name Module Directive Visitors
    /// @brief Methods that process `import` and `export` statements.
    /// @{

    /// @brief Visits an `import` declaration and records source dependencies.
    std::any visitImportStmt(ImportStmtContext* ctx);

    /// @}
};
```

---

### 1.6 Callouts, Diagrams, Math, and Examples

#### Callouts & Lifecycle Tags

Use standard Doxygen alert tags to highlight critical constraints:

```cpp
/// @note Safe to invoke concurrently across worker threads owned by #Driver.
/// @warning The returned pointer is invalidated if the parent #Session is reset.
/// @attention All file paths passed to this API must be normalized to UTF-8.
/// @deprecated Replaced by #Driver::schedule_session(). Will be removed in v0.2.0.
/// @todo Implement library source inclusion for external packages.
/// @bug Does not yet prune unreachable symbols across cyclic imports.
```

#### Embedded Mermaid Diagrams

Docusaurus v3 natively renders `mermaid` code blocks passed through `@xpack/doxygen2docusaurus`. Use them to document compiler passes, state machines, and LSP message flows right next to the code:

````cpp
/// @brief Coordinates concurrent front-end sessions across multiple projects.
///
/// ```mermaid
/// flowchart LR
///     Driver["Driver"] --> S1["Session (Project A)"]
///     Driver --> S2["Session (Project B)"]
///     S1 --> Parser["Lexer & Parser"]
///     Parser --> Diag["Diagnostics"]
/// ```
````

#### Complexity & Code Examples (`@par`)

````cpp
/// @par Complexity
/// Runs in $O(V + E)$ time where $V$ is the number of source files and $E$ is the number of `import` edges.
///
/// @par Example
/// ```cpp
/// jug::core::Driver driver;
/// driver.enqueue_project("F:/project/jug.toml");
/// driver.run_all();
/// ```
````

---

## 2. Linking Classes and Structures Together

Documentation should never exist as isolated islands. Every class, struct, and enum in Juggernyaut **must** be linked to the types it collaborates with so readers can traverse the architecture seamlessly in Docusaurus.

### 2.1 Rules for Cross-Linking Types

1. **Inline Type References (`#` and `@ref`)**:
   * Prefix any class, struct, enum, or member name in prose with `#` (e.g., `#Diagnostic`, `#Session::parse()`, `#SourceSpan`) to force Doxygen to generate a verified link and warn if the symbol is renamed or removed.
   * When custom link text improves readability, use `@ref SymbolName "Custom Label"` (e.g., `@ref JuggernyautParser "the ANTLR4 parser"`).
2. **Bidirectional `@see` Blocks**:
   * Every class and struct **must** end its header comment with one or more `@see` lines pointing to its related types:
     * **Producer ↔ Consumer**: If `Parser` produces an `AstNode`, `Parser` must have `@see AstNode` and `AstNode` must have `@see Parser`.
     * **Container ↔ Element**: If `Session` owns `DiagnosticList` and `Diagnostic` objects, link them in both directions.
     * **Base ↔ Derived / Interface ↔ Implementation**: Always cross-link interfaces, visitors, and concrete implementations.

### 2.2 Example: Bidirectional Linking Between Collaborating Types

```cpp
/// @ingroup CoreDiagnostics
/// @brief Represents a single compiler diagnostic with source location and code.
///
/// Instances of #Diagnostic are created during lexing, parsing, and semantic
/// analysis and aggregated inside a #DiagnosticConsumer owned by the active #Session.
///
/// @see DiagnosticSeverity
/// @see DiagnosticConsumer
/// @see SourceSpan
/// @see Session
struct Diagnostic {
    std::uint32_t code = 1;      ///< @brief Numeric diagnostic code (see `README.md` ranges).
    DiagnosticSeverity severity; ///< @brief Severity level of this diagnostic.
    SourceSpan span;             ///< @brief Byte range in the source buffer where the issue occurred.
};

/// @ingroup CoreDiagnostics
/// @brief Collects and formats #Diagnostic instances emitted by a #Session.
///
/// @see Diagnostic
/// @see Session
class DiagnosticConsumer {
public:
    /// @brief Records a new #Diagnostic into this consumer's buffer.
    /// @param[in] diag The #Diagnostic to store.
    void report(const Diagnostic& diag);
};
```

---

## 3. Grouping Components by Folder

All `.hpp` headers and components inside a directory **must** be grouped together using Doxygen topic groups (`@defgroup`, `@addtogroup`, and `@ingroup`) that mirror the repository's folder hierarchy. `@xpack/doxygen2docusaurus` turns these groups into the **Topics** navigation tree in Docusaurus.

### 3.1 Folder-to-Group Mapping

| Repository Folder | Group ID (`@ingroup`) | Parent Group | Group Title |
| :--- | :--- | :--- | :--- |
| `/core` | `Core` | *(root)* | Core Front-End |
| `/core/data` | `CoreData` | `Core` | Core Data Structures |
| `/core/diagnostics` | `CoreDiagnostics` | `Core` | Core Diagnostics |
| `/core/parser` | `CoreParser` | `Core` | Core Lexer & Parser |
| `/core/session` | `CoreSession` | `Core` | Core Session |
| `/core/driver` | `CoreDriver` | `Core` | Core Front-End Driver |
| `/compiler` | `Compiler` | *(root)* | Compiler & Back-End |
| `/server` | `Server` | *(root)* | Language Server (LSP) |
| `/package-manager` | `PackageManager` | *(root)* | Package Manager |
| `/utility` | `Utility` | *(root)* | Toolchain Utilities |
| `/utility/diag_codes` | `UtilityDiagCodes` | `Utility` | Diagnostic Code Generator |
| `/utility/libraries` | `UtilityLibraries` | `Utility` | Shared Utility Libraries |

---

### 3.2 Defining a Folder Group (`@defgroup`)

Each folder defines its group **once** in that folder's primary `.hpp` umbrella header, linking itself to its parent folder's group via `@ingroup`:

```cpp
/// @defgroup Core Core Front-End
/// @brief Core front-end compilation stages (data, diagnostics, parser, session, and driver).

/// @defgroup CoreParser Core Lexer & Parser
/// @ingroup Core
/// @brief Generates the parser AST and links processing contexts from `.jug` source files.
```

---

### 3.3 Assigning Folder Components to Their Group

Every `.hpp` header file, class, struct, enum, concept, and free function inside a folder **must** belong to that folder's group. You can do this in either of two ways:

#### Option A: Wrap the Header's Declarations in `@addtogroup <FolderGroup> @{ ... @}` (Recommended)

Wrapping the contents of a header inside `@addtogroup ... @{` and `@}` automatically assigns every class, struct, enum, and function in that file to the folder's group without repeating `@ingroup` on every symbol:

```cpp
/// @file
/// @ingroup CoreDriver
/// @brief Declares the concurrent front-end #Driver and its configuration types.

#pragma once

namespace jug::core::driver {

/// @addtogroup CoreDriver
/// @{

/// @brief Options controlling concurrency and target discovery for a #Driver run.
/// @see Driver
struct DriverOptions {
    std::size_t max_threads = 0; ///< @brief Worker thread limit (`0` = hardware concurrency).
};

/// @brief Coordinates concurrent #Session instances across multiple projects.
/// @see DriverOptions
/// @see Session
class Driver {
public:
    /// @brief Initializes a #Driver with the given #DriverOptions.
    /// @param[in] options Configuration settings for the driver.
    explicit Driver(const DriverOptions& options);
};

/// @}

} // namespace jug::core::driver
```

#### Option B: Tag Individual Symbols with `@ingroup <FolderGroup>`

Alternatively, place `@ingroup <FolderGroup>` directly on the doc block of each `.hpp` file, class, struct, or free function:

```cpp
/// @ingroup CoreData
/// @brief Shared symbol table entry produced during semantic analysis.
/// @see ScopeTable
struct SymbolEntry {
    // ...
};
```
