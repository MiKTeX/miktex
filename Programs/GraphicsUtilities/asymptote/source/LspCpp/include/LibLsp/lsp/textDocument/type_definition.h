#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/lsp/out_list.h"

/**
 * The goto type definition request is sent from the client to the server to resolve
 * the type definition location of a symbol at a given text document position.
 *
 * Registration Options: TextDocumentRegistrationOptions
 *
 * Since version 3.6.0
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_typeDefinition,
	lsTextDocumentPositionParams, LocationListEither::Either, "textDocument/typeDefinition");