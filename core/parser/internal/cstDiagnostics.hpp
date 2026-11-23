/**
 * @brief
 * Internal Tree-sitter CST syntax diagnostic builders
**/

#pragma once

#include "common/headers.hpp"

// Tree-sitter
#include "../tree-sitter.hpp"

// Host & Diagnostics
#include "../../data/host/VFS.hpp"
#include "../../data/diagnostics/Diagnostic.hpp"

namespace Parser::Internal::CSTDiagnostics {
    // Host
    using File = Data::Host::File;

    // Diagnostics
    using Position = Data::Diagnostics::Position;
    using Range = Data::Diagnostics::Range;
    using Diagnostic = Data::Diagnostics::Diagnostic;

    Diagnostic buildMissingDiagnostic(ts::Node node, std::string_view source, const File *originFile) ;
    Diagnostic buildUnexpectedDiagnostic(ts::Node node, std::string_view source, const File *originFile) ;
    Diagnostic buildErrorDiagnostic(ts::Node node, std::string_view source, const File *originFile) ;
    Diagnostic buildRangedDiagnostic(ts::Node first, ts::Node last, const File *originFile) ;
}
