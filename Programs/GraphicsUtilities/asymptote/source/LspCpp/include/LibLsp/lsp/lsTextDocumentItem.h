#pragma once

#include "LibLsp/JsonRpc/serializer.h"
#include <string>
#include <vector>
#include "lsDocumentUri.h"

//An item to transfer a text document from the client to the server.
struct lsTextDocumentItem {
	// The text document's URI.
	lsDocumentUri uri;

	// The text document's language identifier.
	std::string languageId;

	// The version number of this document (it will strictly increase after each
	// change, including undo/redo).
	int version = 0;

	// The content of the opened text document.
	std::string text;

	MAKE_SWAP_METHOD(lsTextDocumentItem, uri, languageId, version, text)
};

MAKE_REFLECT_STRUCT(lsTextDocumentItem, uri, languageId, version, text)