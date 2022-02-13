#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "document_symbol.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"


struct FoldingRangeRequestParams {
	/**
	 * The text document.
	 */

	lsTextDocumentIdentifier textDocument;
	MAKE_SWAP_METHOD(FoldingRangeRequestParams, textDocument)
};
MAKE_REFLECT_STRUCT(FoldingRangeRequestParams, textDocument)


struct FoldingRange {
	/**
	 * The zero-based line number from where the folded range starts.
	 */
	 int startLine;

	/**
	 * The zero-based line number where the folded range ends.
	 */
	 int endLine;

	/**
	 * The zero-based character offset from where the folded range starts. If not defined, defaults
	 * to the length of the start line.
	 */
	 int startCharacter;

	/**
	 * The zero-based character offset before the folded range ends. If not defined, defaults to the
	 * length of the end line.
	 */
	 int endCharacter;

	/**
	 * Describes the kind of the folding range such as `comment' or 'region'. The kind
	 * is used to categorize folding ranges and used by commands like 'Fold all comments'. See
	 * FoldingRangeKind for an enumeration of standardized kinds.
	 */
	std::string kind;

	MAKE_SWAP_METHOD(FoldingRange, startLine, endLine, startCharacter, endCharacter, kind)
};
MAKE_REFLECT_STRUCT(FoldingRange,startLine,endLine,startCharacter,endCharacter,kind)


/**
 * The folding range request is sent from the client to the server to return all folding
 * ranges found in a given text document.
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_foldingRange, FoldingRangeRequestParams, std::vector<FoldingRange>, "textDocument/foldingRange");

