#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"
#include "document_symbol.h"

/**
 * The references request is sent from the client to the server to resolve
 * project-wide references for the symbol denoted by the given text document
 * position.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */

DEFINE_REQUEST_RESPONSE_TYPE(td_highlight, lsTextDocumentPositionParams,
	std::vector<lsDocumentHighlight>, "textDocument/documentHighlight");

