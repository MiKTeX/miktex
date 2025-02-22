#pragma once

#include "LibLsp/lsp/lsAny.h"
#include "lsWorkspaceClientCapabilites.h"
#include "lsTextDocumentClientCapabilities.h"

/**
 * Client capabilities specific to the used markdown parser.
 *
 * @since 3.16.0
 */
struct MarkdownClientCapabilities
{
    /**
         * The name of the parser.
         */
    std::string parser;

    /**
         * The version of the parser.
         */
    optional<std::string> version;
    MAKE_SWAP_METHOD(MarkdownClientCapabilities, parser, version)
};
MAKE_REFLECT_STRUCT(MarkdownClientCapabilities, parser, version)

struct lsClientCapabilities
{
    // Workspace specific client capabilities.
    optional<lsWorkspaceClientCapabilites> workspace;

    // Text document specific client capabilities.
    optional<lsTextDocumentClientCapabilities> textDocument;

    /**
        * Window specific client capabilities.
  */
    optional<lsp::Any> window;
    /**
   * Experimental client capabilities.
   */
    optional<lsp::Any> experimental;

    MAKE_SWAP_METHOD(lsClientCapabilities, workspace, textDocument, window, experimental)
};
MAKE_REFLECT_STRUCT(lsClientCapabilities, workspace, textDocument, window, experimental)
