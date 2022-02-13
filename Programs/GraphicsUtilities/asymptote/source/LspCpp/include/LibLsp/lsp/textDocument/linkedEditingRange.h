#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/lsAny.h"
#include "LibLsp/lsp/symbol.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"
#include "LibLsp/lsp/lsRange.h"



struct LinkedEditingRangeParams
{
	lsTextDocumentIdentifier textDocument;
	lsPosition position;

	MAKE_SWAP_METHOD(LinkedEditingRangeParams,
		textDocument,
		position)
};
MAKE_REFLECT_STRUCT(LinkedEditingRangeParams,
	textDocument,
	position)


struct LinkedEditingRanges
{
	/**
 * A list of ranges that can be renamed together. The ranges must have
 * identical length and contain identical text content. The ranges cannot overlap.
 */
  std::vector<lsRange> ranges;

	/**
	 * An optional word pattern (regular expression) that describes valid contents for
	 * the given ranges. If no pattern is provided, the client configuration's word
	 * pattern will be used.
	 */
 
	boost::optional<std::string> wordPattern;
	MAKE_SWAP_METHOD(LinkedEditingRanges,
		ranges,
		wordPattern)
};

MAKE_REFLECT_STRUCT(LinkedEditingRanges,
	ranges,
	wordPattern)
DEFINE_REQUEST_RESPONSE_TYPE(td_linkedEditingRange, LinkedEditingRangeParams,
	boost::optional<std::vector<LinkedEditingRanges >>,"textDocument/linkedEditingRange")