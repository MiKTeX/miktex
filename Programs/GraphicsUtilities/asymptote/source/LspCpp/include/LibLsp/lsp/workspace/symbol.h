#pragma once

#include "LibLsp/lsp/symbol.h"

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

/**
 * The workspace symbol request is sent from the client to the server to
 * list project-wide symbols matching the query string.
 *
 * Registration Options: void
 */

DEFINE_REQUEST_RESPONSE_TYPE(wp_symbol, WorkspaceSymbolParams, std::vector<lsSymbolInformation>, "workspace/symbol");

