/// @file
/// @ingroup CoreDiagnostics
/// @brief Defines core diagnostic data structures, source ranges, severity levels, and helper macros.
///
/// This header provides the foundational #Diagnostics::Diagnostic report model shared
/// across the lexer, parser, AST builder, #Session, and front-end #Driver.

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_DIAGNOSTICS_API

// Generated codes (.diag files)
#include "DiagCodes.hpp"

// Relative origin types
#include "../data/host/VFS.hpp"

/// @defgroup Core Core Front-End
/// @brief Core front-end compilation stages (data, diagnostics, parser, session, and driver).

/// @defgroup CoreDiagnostics Core Diagnostics
/// @ingroup Core
/// @brief Structured diagnostic reporting, ANTLR4/AST error conversion, and lifecycle management.
///
/// ```mermaid
/// flowchart LR
///     ANTLR["ANTLR4 Lexer / Parser"] -->|antlr.hpp| Diag["Diagnostics::Diagnostic"]
///     AST["AST Builders / Visitors"] -->|astGen.hpp| Diag
///     Diag -->|Driver Hooks| Mgr["Diagnostics::DiagnosticsManager"]
///     Mgr -->|onFileFlush / onComponentFlush| Consumer["CLI / LSP Server"]
/// ```
/// @{

/// @brief Declares a new #Diagnostics::Diagnostic variable and populates its severity, numeric code, and formatted message.
///
/// @param[out] DIAG     Identifier name for the newly declared #Diagnostics::Diagnostic local variable.
/// @param[in]  SEVERITY Enumerator name from #Diagnostics::Severity (e.g., `Error`, `Warning`, `Info`, `Hint`).
/// @param[in]  NUM      Numeric diagnostic code matching a generated `CODE_<NUM>` macro in `DiagCodes.hpp`.
/// @param[in]  ...      Optional format arguments forwarded to `CODE_<NUM>(...)`.
///
/// @see USE_CODE
/// @see Diagnostics::Diagnostic
/// @see Diagnostics::Severity
#define SET_CODE(DIAG, SEVERITY, NUM, ...) \
    Diagnostics::Diagnostic DIAG; \
    (DIAG).severity = Diagnostics::Severity::SEVERITY; \
    (DIAG).code = (NUM); \
    (DIAG).message = CODE_##NUM __VA_OPT__((__VA_ARGS__))

/// @brief Populates an existing #Diagnostics::Diagnostic instance with a severity, numeric code, and formatted message.
///
/// @param[in,out] DIAG     Existing #Diagnostics::Diagnostic instance to mutate.
/// @param[in]     SEVERITY Enumerator name from #Diagnostics::Severity (e.g., `Error`, `Warning`, `Info`, `Hint`).
/// @param[in]     NUM      Numeric diagnostic code matching a generated `CODE_<NUM>` macro in `DiagCodes.hpp`.
/// @param[in]     ...      Optional format arguments forwarded to `CODE_<NUM>(...)`.
///
/// @see SET_CODE
/// @see Diagnostics::Diagnostic
/// @see Diagnostics::Severity
#define USE_CODE(DIAG, SEVERITY, NUM, ...) \
    (DIAG).severity = Diagnostics::Severity::SEVERITY; \
    (DIAG).code = (NUM); \
    (DIAG).message = CODE_##NUM __VA_OPT__((__VA_ARGS__))

/// @}

namespace Diagnostics {

    /// @addtogroup CoreDiagnostics
    /// @{

    // Basic data types
    /// @brief Alias for the virtual file system's host file descriptor (`Data::Host::File`).
    /// @see Diagnostic
    using File = Data::Host::File;

    /// @brief Alias for a unique host file identifier (`Data::Host::FileId`).
    /// @see File
    using FileId = Data::Host::FileId;

    // Event types
    /// @brief Callback signature for parameterless lifecycle notifications.
    using GenericEvent = std::function<void()>;

    /// @brief Callback signature for file-scoped lifecycle events.
    /// @see File
    /// @see DiagnosticsManager
    using FileEvent = std::function<void(const Data::Host::File*)>;

    // Line/column data
    /// @brief Zero-based line and UTF-8 character column offset within a source #File.
    ///
    /// @see Range
    /// @see Diagnostic
    struct Position {
        uint32_t line = 0;      ///< @brief Zero-based line index within the source file.
        uint32_t character = 0; ///< @brief Zero-based character offset within #line.

        /// @brief Compares two #Position instances for exact line and character equality.
        bool operator==(const Position&) const = default;
    };

    // The exact position within the final source
    /// @brief Contiguous source span bounded by a starting and ending #Position.
    ///
    /// @see Position
    /// @see Diagnostic
    struct Range {
        Position start; ///< @brief Inclusive start #Position of the span.
        Position end;   ///< @brief Exclusive end #Position of the span.
    };

    // Report severity
    /// @brief Indicates the impact level of a #Diagnostic (aligned with LSP `DiagnosticSeverity` values).
    ///
    /// @see Diagnostic
    /// @see SET_CODE
    /// @see USE_CODE
    enum Severity {
	    Error = 1,   ///< @brief Fatal compilation or syntax error (`1`).
    	Warning = 2, ///< @brief Suspicious construct that does not halt compilation (`2`).
	    Info = 3,    ///< @brief Informational message (`3`).
    	Hint = 4     ///< @brief Non-intrusive stylistic or refactoring hint (`4`).
    };

    // A call stack trace layer!
    /*struct Trace {
        std::string uri;
        Range position;
        std::string description;
    };*/
    // Extra information!
    /*struct RelatedInformation {
        std::string uri;
        Range position;
        Severity severity;
        std::string message;
    };*/

    // A report
    /// @brief Represents a single compiler or language-server diagnostic report.
    ///
    /// A #Diagnostic may either be attached to a specific source #File (with a source #Range)
    /// or represent a global component diagnostic when #file is `nullptr`.
    ///
    /// @see Position
    /// @see Range
    /// @see Severity
    /// @see DiagnosticsManager
    /// @see antlrSyntaxErrorToDiagnostic
    /// @see getGenTokenDiagnostic
    struct Diagnostic {
        // Source info
        const File* file = nullptr; ///< @brief Originating source #File, or `nullptr` for component-wide diagnostics.
        Range range;                ///< @brief Source #Range within #file where the issue occurred.

        // Content
        std::string message = "";           ///< @brief Human-readable diagnostic message text.
        int code = 1;                       ///< @brief Numeric diagnostic code (see `DiagCodes.hpp`).
        Severity severity = Severity::Hint; ///< @brief Impact level of this diagnostic.
        // std::vector<Trace> stack;
        // std::vector<RelatedInformation> related;
    };

    /// @brief Read-only callback signature invoked when a #Diagnostic is emitted.
    /// @see Diagnostic
    /// @see DiagnosticsManager
    using Event = std::function<void(const Diagnostic&)>;

    /// @brief Mutating callback signature used internally to enrich an emitted #Diagnostic in place.
    /// @see Diagnostic
    using ModifiableEvent = std::function<void(Diagnostic&)>; // Only for internal manipulation

    /*struct FileDiagnostics {
        std::string uri;
        std::vector<Diagnostic> diagnostics;
    };*/

    /// @}
}
