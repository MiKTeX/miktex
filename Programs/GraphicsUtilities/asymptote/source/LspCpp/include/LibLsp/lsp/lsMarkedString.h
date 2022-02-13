#pragma once

#include "LibLsp/JsonRpc/serializer.h"

#include <string>
#include <vector>


// MarkedString can be used to render human readable text. It is either a
// markdown string or a code-block that provides a language and a code snippet.
// The language identifier is sematically equal to the optional language
// identifier in fenced code blocks in GitHub issues. See
// https://help.github.com/articles/creating-and-highlighting-code-blocks/#syntax-highlighting
//
// The pair of a language and a value is an equivalent to markdown:
// ```${language}
// ${value}
// ```
//
// Note that markdown strings will be sanitized - that means html will be
// escaped.
struct lsMarkedString {
	boost::optional<std::string> language;
	std::string value;
};

struct MarkupContent {
	/**
	 * The type of the Markup.
	 */

	 std::string kind;

	/**
	 * The content itself.
	 */
	
	 std::string value;

	 MAKE_SWAP_METHOD(MarkupContent, kind, value);
};
MAKE_REFLECT_STRUCT(MarkupContent,kind,value);

void Reflect(Writer& visitor, lsMarkedString& value);
void Reflect(Reader& visitor, lsMarkedString& value);
