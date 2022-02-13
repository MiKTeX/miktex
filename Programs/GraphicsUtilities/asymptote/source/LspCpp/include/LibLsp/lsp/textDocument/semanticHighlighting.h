#pragma once
#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsVersionedTextDocumentIdentifier.h"

#include "LibLsp/JsonRpc/NotificationInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

/**
 * Represents a semantic highlighting information that has to be applied on a specific line of the text document.
 */

struct SemanticHighlightingInformation {
	/**
	 * The zero-based line position in the text document.
	 */
	int line = 0;

	/**
	 * A base64 encoded string representing every single highlighted ranges in the line with its start position, length
	 * and the "lookup table" index of of the semantic highlighting <a href="https://manual.macromates.com/en/language_grammars">
	 * TextMate scopes</a>. If the {@code tokens} is empty or not defined, then no highlighted positions are available for the line.
	 */
	 std::string tokens;

	 MAKE_SWAP_METHOD(SemanticHighlightingInformation, line, tokens)
};

MAKE_REFLECT_STRUCT(SemanticHighlightingInformation, line, tokens);

/**
 * Parameters for the semantic highlighting (server-side) push notification.
 */

struct SemanticHighlightingParams {
	/**
	 * The text document that has to be decorated with the semantic highlighting information.
	 */
	
	 lsVersionedTextDocumentIdentifier textDocument;

	/**
	 * An array of semantic highlighting information.
	 */

	 std::vector<SemanticHighlightingInformation> lines;
	
	 MAKE_SWAP_METHOD(SemanticHighlightingParams, textDocument, lines)
	
};
MAKE_REFLECT_STRUCT(SemanticHighlightingParams, textDocument, lines);
/**
	 * The {@code textDocument/semanticHighlighting} notification is pushed from the server to the client
	 * to inform the client about additional semantic highlighting information that has to be applied
	 * on the text document. It is the server's responsibility to decide which lines are included in
	 * the highlighting information. In other words, the server is capable of sending only a delta
	 * information. For instance, after opening the text document ({@code DidOpenTextDocumentNotification})
	 * the server sends the semantic highlighting information for the entire document, but if the server
	 * receives a {@code DidChangeTextDocumentNotification}, it pushes the information only about
	 * the affected lines in the document.
	 *
	 * <p>
	 * <b>Note:</b> the <a href=
	 * "https://github.com/Microsoft/vscode-languageserver-node/pull/367">{@code textDocument/semanticHighlighting}
	 * language feature</a> is not yet part of the official LSP specification.
	 */
DEFINE_NOTIFICATION_TYPE(Notify_semanticHighlighting, SemanticHighlightingParams, "textDocument/semanticHighlighting");