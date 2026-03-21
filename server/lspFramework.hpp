/**
 * @brief
 * lsp-framework wrapper
**/


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

#include <lsp/messages.h>
#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>

// --- Restore warnings back to normal for your code ---
#ifdef _MSC_VER
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
