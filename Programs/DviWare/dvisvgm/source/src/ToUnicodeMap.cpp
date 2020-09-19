/*************************************************************************
** ToUnicodeMap.cpp                                                     **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#include <limits>
#include "ToUnicodeMap.hpp"
#include "Unicode.hpp"

using namespace std;


/** Adds valid but random mappings for all missing character indexes.
 *  If a font's cmap table doesn't provide Unicode mappings for some
 *  glyphs in the font, it's necessary to fill the gaps in order to
 *  handle all characters correctly. This functions assumes that the
 *  characters are numbered from 1 to maxIndex.
 *  @param[in] maxIndex largest character index to consider
 *  @return true on success */
bool ToUnicodeMap::addMissingMappings (uint32_t maxIndex) {
	bool success=true;
	// collect Unicode points already in assigned
	NumericRanges<uint32_t> codepoints;
	for (size_t i=0; i < numRanges() && success; i++)
		codepoints.addRange(rangeAt(i).minval(), rangeAt(i).maxval());
	// fill unmapped ranges
	if (empty()) // no Unicode mapping present at all?
		success = fillRange(1, maxIndex, 1, codepoints, true);
	else {   // (partial) Unicode mapping present?
		success = fillRange(1, rangeAt(0).min()-1, rangeAt(0).minval()-1, codepoints, false);
		for (size_t i=0; i < numRanges()-1 && success; i++)
			success = fillRange(rangeAt(i).max()+1, rangeAt(i+1).min()-1, rangeAt(i).maxval()+1, codepoints, true);
		if (success)
			success = fillRange(rangeAt(numRanges()-1).max()+1, maxIndex, rangeAt(numRanges()-1).maxval()+1, codepoints, true);
	}
	return success;
}


/** Checks if a given codepoint is valid and unused. Otherwise, try to find an alternative.
 * @param[in,out] ucp codepoint to fix
 * @param[in] used_codepoints codepoints already in use
 * @param[in] ascending if true, increase ucp to look for valid/unused codepoints
 * @return true on success */
static bool fix_codepoint (uint32_t &ucp, const NumericRanges<uint32_t> &used_codepoints, bool ascending) {
	uint32_t start = ucp;
	while (!Unicode::isValidCodepoint(ucp) && used_codepoints.valueExists(ucp)) {
		if (ascending)
			ucp = (ucp == numeric_limits<uint32_t>::max()) ? 0 : ucp+1;
		else
			ucp = (ucp == 0) ? numeric_limits<uint32_t>::max() : ucp-1;
		if (ucp == start) // no free Unicode point found
			return false;
	}
	return true;
}


static bool is_less_or_equal (uint32_t a, uint32_t b) {return a <= b;}
static bool is_greater_or_equal (uint32_t a, uint32_t b) {return a >= b;}


/** Adds index to Unicode mappings for a given range of character indexes.
 *  @param[in] minIndex lower bound of range to fill
 *  @param[in] maxIndex upper bound of range to fill
 *  @param[in] ucp first Unicode point to add (if possible)
 *  @param[in,out] used_ucps Unicode points already in use
 *  @param[in] ascending if true, fill range from lower to upper bound
 *  @return true on success */
bool ToUnicodeMap::fillRange (uint32_t minIndex, uint32_t maxIndex, uint32_t ucp, NumericRanges<uint32_t> &used_ucps, bool ascending) {
	if (minIndex <= maxIndex) {
		uint32_t first=minIndex, last=maxIndex;
		int inc=1;
		bool (*cmp)(uint32_t, uint32_t) = is_less_or_equal;
		if (!ascending) {
			swap(first, last);
			inc = -1;
			cmp = is_greater_or_equal;
		}
		for (uint32_t i=first; cmp(i, last); i += inc) {
			if (!fix_codepoint(ucp, used_ucps, ascending))
				return false;
			else {
				addRange(i, i, ucp);
				used_ucps.addRange(ucp);
				ucp += inc;  // preferred Unicode point for the next character of the current range
			}
		}
	}
	return true;
}