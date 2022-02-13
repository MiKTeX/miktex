#pragma once

#include "LibLsp/JsonRpc/serializer.h"
#include "lsDocumentUri.h"
//Text documents are identified using a URI.On the protocol level,
//URIs are passed as strings.The corresponding JSON structure looks like this:
struct lsTextDocumentIdentifier {
	/**
	 * The text document's URI.
	 */
	lsDocumentUri uri;
    MAKE_SWAP_METHOD(lsTextDocumentIdentifier, uri)
};
MAKE_REFLECT_STRUCT(lsTextDocumentIdentifier, uri)