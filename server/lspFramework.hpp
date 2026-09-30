/**
 * @brief
 * lsp-framework wrapper
**/

#pragma once

// lsp-framework
// --- Turn off warnings for the LSP framework headers ---
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4456 4457 4458 4459 4834 6031)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wshadow"
    #pragma GCC diagnostic ignored "-Wunused-result"
#endif

#include <lsp/types.h>
#include <lsp/error.h>
#include <lsp/io/socket.h>
#include <lsp/io/standard_io.h>
#include <lsp/messages.h>
#include <lsp/protocol_version.h>
#include <lsp/server_endpoint.h>

// --- Restore warnings back to normal for your code ---
#ifdef _MSC_VER
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
