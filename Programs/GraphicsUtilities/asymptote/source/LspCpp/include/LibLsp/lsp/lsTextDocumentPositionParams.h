#pragma once

#include "LibLsp/JsonRpc/message.h"

#include "lsTextDocumentIdentifier.h"
#include "lsPosition.h"

/**
 * A parameter literal used in requests to pass a text document and a position inside that document.
 */
struct lsTextDocumentPositionParams {
	// The text document.
	lsTextDocumentIdentifier textDocument;

	// The position inside the text document.
	lsPosition position;

	/**
	 * Legacy property to support protocol version 1.0 requests.
	 */
	boost::optional<lsDocumentUri> uri;
	
   MAKE_SWAP_METHOD(lsTextDocumentPositionParams, textDocument, position, uri);
	
};
MAKE_REFLECT_STRUCT(lsTextDocumentPositionParams, textDocument, position, uri);
