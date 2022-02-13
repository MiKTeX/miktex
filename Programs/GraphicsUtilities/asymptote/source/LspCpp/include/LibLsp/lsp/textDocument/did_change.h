#pragma once


#include "LibLsp/JsonRpc/NotificationInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsVersionedTextDocumentIdentifier.h"
#include "LibLsp/lsp/lsRange.h"
#include "LibLsp/lsp/lsDocumentUri.h"
struct lsTextDocumentContentChangeEvent {
	// The range of the document that changed.
	boost::optional<lsRange> range;
	// The length of the range that got replaced.
	boost::optional<int> rangeLength;
	// The new text of the range/document.
	std::string text;

	MAKE_SWAP_METHOD(lsTextDocumentContentChangeEvent, range, rangeLength, text);
};
MAKE_REFLECT_STRUCT(lsTextDocumentContentChangeEvent, range, rangeLength, text);


struct lsTextDocumentDidChangeParams {
	lsVersionedTextDocumentIdentifier textDocument;
	std::vector<lsTextDocumentContentChangeEvent> contentChanges;

	/**
	 * Legacy property to support protocol version 1.0 requests.
	 */
	
	boost::optional<lsDocumentUri>  uri;
	
	void swap(lsTextDocumentDidChangeParams& arg) noexcept
	{
		uri.swap(arg.uri);
		contentChanges.swap(arg.contentChanges);
		textDocument.swap(arg.textDocument);
	}
};
MAKE_REFLECT_STRUCT(lsTextDocumentDidChangeParams,
	textDocument,
	contentChanges, uri);

/**
 * The document change notification is sent from the client to the server to
 * signal changes to a text document.
 *
 * Registration Options: TextDocumentChangeRegistrationOptions
 */
DEFINE_NOTIFICATION_TYPE(Notify_TextDocumentDidChange, lsTextDocumentDidChangeParams, "textDocument/didChange");

