#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
#include "LibLsp/lsp/lsRange.h"
#include "LibLsp/lsp/location_type.h"
#include "LibLsp/lsp/lsCommand.h"
#include "LibLsp/lsp/lsTextEdit.h"
#include "LibLsp/lsp/lsAny.h"

namespace TextDocumentInlayHint
{

struct Params
{
    // The text document
    lsTextDocumentIdentifier textDocument;
    // The visible document range for which inlay hints should be computed.
    lsRange range;

    MAKE_SWAP_METHOD(Params, textDocument, range)
};

}; // namespace TextDocumentInlayHint
MAKE_REFLECT_STRUCT(TextDocumentInlayHint::Params, textDocument, range);

struct lsInlayHintLabelPart
{

    /**
     * The value of this label part.
     */
    std::string value;

    /**
	 * The tooltip text when you hover over this label part. Depending on
	 * the client capability `inlayHint.resolveSupport` clients might resolve
	 * this property late using the resolve request.
	 */
    optional<std::string> tooltip;

    /**
	 * An optional source code location that represents this
	 * label part.
	 *
	 * The editor will use this location for the hover and for code navigation
	 * features: This part will become a clickable link that resolves to the
	 * definition of the symbol at the given location (not necessarily the
	 * location itself), it shows the hover that shows at the given location,
	 * and it shows a context menu with further code navigation commands.
	 *
	 * Depending on the client capability `inlayHint.resolveSupport` clients
	 * might resolve this property late using the resolve request.
	 */
    optional<lsLocation> location;

    /**
	 * An optional command for this label part.
	 *
	 * Depending on the client capability `inlayHint.resolveSupport` clients
	 * might resolve this property late using the resolve request.
	 */
    optional<lsCommand<lsp::Any>> command;

    MAKE_SWAP_METHOD(lsInlayHintLabelPart, value, tooltip, location, command)
};

MAKE_REFLECT_STRUCT(lsInlayHintLabelPart, value, tooltip, location, command);

enum class lsInlayHintKind
{

    // An inlay hint that for a type annotation.
    Type = 1,

    // An inlay hint that is for a parameter.
    Parameter = 2
};

MAKE_REFLECT_TYPE_PROXY(lsInlayHintKind);

/**
 * a inlay hint is displayed in the editor right next to normal code, it is only readable text
 * that acts like a hint, for example parameter names in function calls are displayed in editors
 * as inlay hints
 */
struct lsInlayHint
{

    /**
    * The position of this hint.
    *
    * If multiple hints have the same position, they will be shown in the order
    * they appear in the response.
    */
    lsPosition position;

    /**
	 * The label of this hint. A human readable string or an array of
	 * InlayHintLabelPart label parts.
	 *
	 * *Note* that neither the string nor the label part can be empty.
	 */
    std::string label;

    /**
	 * The kind of this hint. Can be omitted in which case the client
	 * should fall back to a reasonable default.
	 */
    optional<lsInlayHintKind> kind;

    /**
	 * Optional text edits that are performed when accepting this inlay hint.
	 *
	 * *Note* that edits are expected to change the document so that the inlay
	 * hint (or its nearest variant) is now part of the document and the inlay
	 * hint itself is now obsolete.
	 *
	 * Depending on the client capability `inlayHint.resolveSupport` clients
	 * might resolve this property late using the resolve request.
	 */
    optional<std::vector<lsTextEdit>> textEdits;

    /**
	 * The tooltip text when you hover over this item.
	 *
	 * Depending on the client capability `inlayHint.resolveSupport` clients
	 * might resolve this property late using the resolve request.
	 */
    optional<std::string> tooltip;

    /**
	 * Render padding before the hint.
	 *
	 * Note: Padding should use the editor's background color, not the
	 * background color of the hint itself. That means padding can be used
	 * to visually align/separate an inlay hint.
	 */
    optional<bool> paddingLeft;

    /**
	 * Render padding after the hint.
	 *
	 * Note: Padding should use the editor's background color, not the
	 * background color of the hint itself. That means padding can be used
	 * to visually align/separate an inlay hint.
	 */
    optional<bool> paddingRight;

    /**
	 * A data entry field that is preserved on an inlay hint between
	 * a `textDocument/inlayHint` and a `inlayHint/resolve` request.
	 */
    optional<lsp::Any> data;

    MAKE_SWAP_METHOD(lsInlayHint, position, label, kind, textEdits, tooltip, paddingLeft, paddingRight, data)
};

MAKE_REFLECT_STRUCT(lsInlayHint, position, label, kind, textEdits, tooltip, paddingLeft, paddingRight, data)

DEFINE_REQUEST_RESPONSE_TYPE(
    td_inlayHint, TextDocumentInlayHint::Params, std::vector<lsInlayHint>, "textDocument/inlayHint"
);

/**
 * The document link resolve request is sent from the client to the server to resolve the target of a given document link.
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_inlayHintResolve, lsInlayHint, lsInlayHint, "inlayHint/resolve");
