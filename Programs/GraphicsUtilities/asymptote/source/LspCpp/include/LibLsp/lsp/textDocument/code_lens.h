#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

struct lsDocumentCodeLensParams {
	
/**
 * The document to request code lens for.
 */
  lsTextDocumentIdentifier textDocument;

  MAKE_SWAP_METHOD(lsDocumentCodeLensParams, textDocument);
};
MAKE_REFLECT_STRUCT(lsDocumentCodeLensParams, textDocument);




struct lsCodeLens {
	// The range in which this code lens is valid. Should only span a single line.
	lsRange range;
	// The command this code lens represents.
	boost::optional<lsCommandWithAny> command;
	// A data entry field that is preserved on a code lens item between
	// a code lens and a code lens resolve request.
	boost::optional< lsp::Any> data;

	MAKE_SWAP_METHOD(lsCodeLens, range, command, data)
};
MAKE_REFLECT_STRUCT(lsCodeLens, range, command, data)



/**
 * The code lens request is sent from the client to the server to compute
 * code lenses for a given text document.
 *
 * Registration Options: CodeLensRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_codeLens, lsDocumentCodeLensParams, std::vector<lsCodeLens>, "textDocument/codeLens")

