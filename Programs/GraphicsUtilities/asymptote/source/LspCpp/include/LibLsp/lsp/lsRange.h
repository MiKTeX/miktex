#pragma once

#include "LibLsp/JsonRpc/serializer.h"



#include <string>
#include <vector>
#include "lsPosition.h"
//A range in a text document expressed as(zero - based) startand end positions.
//A range is comparable to a selection in an editor.Therefore the end position is exclusive.
//If you want to specify a range that contains a line including the line ending character(s)
//then use an end position denoting the start of the next line.
struct lsRange {
	lsRange();
	lsRange(lsPosition start, lsPosition end);

	bool operator==(const lsRange& other) const;
	bool operator<(const lsRange& other) const;
	/**
	 * The range's start position.
	 */
	lsPosition start;
	/**
	 * The range's end position.
	 */
	lsPosition end;
	std::string ToString()const;
	MAKE_SWAP_METHOD(lsRange, start, end)
};

MAKE_REFLECT_STRUCT(lsRange, start, end)