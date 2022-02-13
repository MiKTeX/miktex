#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
#include "LibLsp/lsp/lsRange.h"
#include "documentColor.h"
#include "LibLsp/lsp/lsTextEdit.h"


struct ColorPresentationParams {

	/**
   * The text document.
   */
	lsTextDocumentIdentifier textDocument;

	/**
	 * The range in the document where this color appers.
	 */

	lsRange range;

	/**
	 * The actual color value for this color range.
	 */

	TextDocument::Color color;
	MAKE_SWAP_METHOD(ColorPresentationParams, textDocument, range, color)
};
MAKE_REFLECT_STRUCT(ColorPresentationParams, textDocument, range, color)


struct ColorPresentation {
	/**
	 * The label of this color presentation. It will be shown on the color
	 * picker header. By default this is also the text that is inserted when selecting
	 * this color presentation.
	 */

	 std::string label;

	/**
	 * An edit which is applied to a document when selecting
	 * this presentation for the color.  When `null` the label is used.
	 */
	 lsTextEdit textEdit;

	/**
	 * An optional array of additional text edits that are applied when
	 * selecting this color presentation. Edits must not overlap with the main edit nor with themselves.
	 */
	std::vector<lsTextEdit> additionalTextEdits;
	MAKE_SWAP_METHOD(ColorPresentation, label, textEdit, additionalTextEdits)
};
MAKE_REFLECT_STRUCT(ColorPresentation, label, textEdit, additionalTextEdits)



DEFINE_REQUEST_RESPONSE_TYPE(td_colorPresentation,
	ColorPresentationParams, std::vector<ColorPresentation>, "textDocument/colorPresentation")