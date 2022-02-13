#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
#include "LibLsp/lsp/lsRange.h"
#include <vector>
struct DocumentColorParams {
	/**
	 * The text document.
	 */

	 lsTextDocumentIdentifier textDocument;
	 MAKE_SWAP_METHOD(DocumentColorParams, textDocument);
};
MAKE_REFLECT_STRUCT(DocumentColorParams, textDocument);

/**
 * The document color request is sent from the client to the server to list all color references found in a given text
 * document. Along with the range, a color value in RGB is returned.
 *
 * Clients can use the result to decorate color references in an editor. For example:
 *  - Color boxes showing the actual color next to the reference
 *  - Show a color picker when a color reference is edited
 *
 * Since version 3.6.0
 */

namespace TextDocument {
	struct  Color {
		/**
		 * The red component of this color in the range [0-1].
		 */
		double red = 0;

		/**
		 * The green component of this color in the range [0-1].
		 */
		double green = 0;

		/**
		 * The blue component of this color in the range [0-1].
		 */
		double blue = 0;

		/**
		 * The alpha component of this color in the range [0-1].
		 */
		double alpha = 0;
		MAKE_SWAP_METHOD(TextDocument::Color, red, green, blue, alpha)
	};
}
MAKE_REFLECT_STRUCT(TextDocument::Color, red, green, blue, alpha)


struct ColorInformation {
	/**
	 * The range in the document where this color appers.
	 */

	lsRange range;

	/**
	 * The actual color value for this color range.
	 */

	TextDocument::Color color;
	MAKE_SWAP_METHOD(ColorInformation, range, color)
};
MAKE_REFLECT_STRUCT(ColorInformation,range,color)

DEFINE_REQUEST_RESPONSE_TYPE(td_documentColor, DocumentColorParams,std::vector<ColorInformation>, "textDocument/documentColor");