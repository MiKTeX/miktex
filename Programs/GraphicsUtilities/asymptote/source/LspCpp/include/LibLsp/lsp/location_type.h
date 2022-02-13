#pragma once

#include "lsDocumentUri.h"
#include "lsRange.h"
//Represents a location inside a resource, such as a line inside a text file.
struct lsLocation {
	lsLocation();
	lsLocation(lsDocumentUri uri, lsRange range);

	bool operator==(const lsLocation& other) const;
	bool operator<(const lsLocation& o) const;

	lsDocumentUri uri;
	lsRange range;
	MAKE_SWAP_METHOD(lsLocation, uri, range)
};
MAKE_REFLECT_STRUCT(lsLocation, uri, range)



struct LinkLocation :public lsLocation
{
	std::string displayName;
	std::string kind;
	MAKE_REFLECT_STRUCT(LinkLocation, uri, range, displayName, kind)
};
MAKE_REFLECT_STRUCT(LinkLocation, uri, range, displayName,kind)

//Represents a link between a sourceand a target location.
struct LocationLink
{
	/**
	 * Span of the origin of this link.
	 *
	 * Used as the underlined span for mouse interaction. Defaults to the word range at
	 * the mouse position.
	 */
	boost::optional<lsRange>  originSelectionRange;

	/**
	 * The target resource identifier of this link.
	 */

	lsDocumentUri targetUri;

	/**
	 * The full target range of this link. If the target for example is a symbol then target range is the
	 * range enclosing this symbol not including leading/trailing whitespace but everything else
	 * like comments. This information is typically used to highlight the range in the editor.
	 */

	lsRange targetRange;

	/**
	 * The range that should be selected and revealed when this link is being followed, e.g the name of a function.
	 * Must be contained by the the `targetRange`. See also `DocumentSymbol#range`
	 */

	lsRange targetSelectionRange;

	MAKE_SWAP_METHOD(LocationLink, originSelectionRange, targetUri, targetRange, targetSelectionRange);
};
MAKE_REFLECT_STRUCT(LocationLink, originSelectionRange, targetUri, targetRange, targetSelectionRange);