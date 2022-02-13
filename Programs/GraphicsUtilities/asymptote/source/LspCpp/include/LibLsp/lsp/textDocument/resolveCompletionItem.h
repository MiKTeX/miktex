#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include <regex>
#include "LibLsp/lsp/lsp_completion.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"
#include "completion.h"

/**
 * The request is sent from the client to the server to resolve additional
 * information for a given completion item.
 */
DEFINE_REQUEST_RESPONSE_TYPE(completionItem_resolve, lsCompletionItem, lsCompletionItem, "completionItem/resolve");





