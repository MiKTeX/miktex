#pragma once

#include "LibLsp/JsonRpc/serializer.h"



#include <string>
#include <vector>

//Position in a text document expressed as zero - based line and zero - based character offset.
//A position is between two characters like an ¡®insert¡¯ cursor in a editor.Special values like
//for example - 1 to denote the end of a line are not supported.
struct lsPosition {
	lsPosition();
	lsPosition(int line, int character);

	bool operator==(const lsPosition& other) const;
	bool operator<(const lsPosition& other) const;

	std::string ToString() const;

	/**
	 * Line position in a document (zero-based).
	 */
	// Note: these are 0-based.
	unsigned line = 0;
	/**
	 * Character offset on a line in a document (zero-based). Assuming that
	 * the line is represented as a string, the `character` value represents
	 * the gap between the `character` and `character + 1`.
	 *
	 * If the character value is greater than the line length it defaults back
	 * to the line length.
	 */
	unsigned character = 0;
	static const lsPosition kZeroPosition;

	MAKE_SWAP_METHOD(lsPosition, line, character)
};
MAKE_REFLECT_STRUCT(lsPosition, line, character);