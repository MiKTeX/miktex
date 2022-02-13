#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsPosition.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"

struct SelectionRangeParams {
	/**
	 * The text document.
	 */

	lsTextDocumentIdentifier textDocument;

	/**
	 * The positions inside the text document.
	 */

	std::vector<lsPosition> positions;
	
	MAKE_SWAP_METHOD(SelectionRangeParams, textDocument, positions)
	
};
MAKE_REFLECT_STRUCT(SelectionRangeParams, textDocument, positions)

struct SelectionRange {
	/**
	 * The [range](#Range) of this selection range.
	 */

	lsRange range;

	/**
	 * The parent selection range containing this range. Therefore `parent.range` must contain `this.range`.
	 */
	boost::optional<SelectionRange*> parent;
	MAKE_SWAP_METHOD(SelectionRange, range, parent)
};

extern  void Reflect(Reader& visitor, boost::optional<SelectionRange*>& value);
extern void Reflect(Writer& visitor, SelectionRange* value);

MAKE_REFLECT_STRUCT(SelectionRange,range,parent)
/**
 * The {@code textDocument/selectionRange} request is sent from the client to the server to return
 * suggested selection ranges at an array of given positions. A selection range is a range around
 * the cursor position which the user might be interested in selecting.
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_selectionRange, SelectionRangeParams, std::vector<SelectionRange>, "textDocument/selectionRange");

