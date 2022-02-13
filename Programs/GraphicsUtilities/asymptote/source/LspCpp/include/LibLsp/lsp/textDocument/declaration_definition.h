#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsTextDocumentPositionParams.h"
#include "LibLsp/lsp/out_list.h"


/**
 * The go to declaration request is sent from the client to the server to resolve
 * the declaration location of a symbol at a given text document position.
 *
 * Registration Options: TextDocumentRegistrationOptions
 *
 * Since version 3.14.0
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_declaration, lsTextDocumentPositionParams, LocationListEither::Either, "textDocument/declaration");

/**
 * The goto definition request is sent from the client to the server to resolve
 * the definition location of a symbol at a given text document position.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_definition, lsTextDocumentPositionParams, LocationListEither::Either, "textDocument/definition");


