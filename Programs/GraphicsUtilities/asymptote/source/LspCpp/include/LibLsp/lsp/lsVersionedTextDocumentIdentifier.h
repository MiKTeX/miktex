#pragma once

#include "LibLsp/JsonRpc/serializer.h"

#include "LibLsp/JsonRpc/message.h"
#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"

struct lsVersionedTextDocumentIdentifier
{
	lsDocumentUri uri;
	// The version number of this document.  number | null
	boost::optional<int> version;

	lsTextDocumentIdentifier AsTextDocumentIdentifier() const;

	MAKE_SWAP_METHOD(lsVersionedTextDocumentIdentifier, uri, version)
};
MAKE_REFLECT_STRUCT(lsVersionedTextDocumentIdentifier, uri, version)

/**
 * The version number of this document. If an optional versioned text document
 * identifier is sent from the server to the client and the file is not
 * open in the editor (the server has not received an open notification
 * before) the server can send `null` to indicate that the version is
 * known and the content on disk is the master (as specified with document
 * content ownership).
 *
 * The version number of a document will increase after each change,
 * including undo/redo. The number doesn't need to be consecutive.
 */
using   lsOptionalVersionedTextDocumentIdentifier = lsVersionedTextDocumentIdentifier;