/*************************************************************************
** CmapTable.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2023 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <algorithm>
#include "CmapTable.hpp"
#include "TTFWriter.hpp"
#include "../Font.hpp"
#include "../utility.hpp"

using namespace std;
using namespace ttf;

void CmapTable::write (ostream &os) const {
	writeUInt16(os, 0);  // version
	writeUInt16(os, 2);  // number of encoding tables

	const RangeMap &charmap = ttfWriter()->getUnicodeCharMap();
	bool isUCS2 = (charmap.maxKey() <= 0xFFFF);
	uint32_t offset = 4+2*8;  // offset to subtable

	writeUInt16(os, 0);  // platform ID 0 = Unicode
	writeUInt16(os, isUCS2 ? 3 : 4);  // encoding ID (Unicode 2.0, UCS-2 or UCS-4)
	writeUInt32(os, offset);

	writeUInt16(os, 3);  // platform ID 3 = Windows
	writeUInt16(os, isUCS2 ? 1 : 10);  // encoding ID (UCS-2 or UCS-4)
	writeUInt32(os, offset);  // platform ID 0 = Unicode

	if (isUCS2)
		writeSubTableFormat4(os, charmap);
	else
		writeSubTableFormat12(os, charmap);
}


/** Writes mapping data in subtable format 0 (byte encoding table). */
void CmapTable::writeSubTableFormat0 (ostream &os, const RangeMap &charmap) const {
	writeUInt16(os, 0);        // subtable format
	writeUInt16(os, 3*2+256);  // table length
	writeUInt16(os, 0);        // language: none
	vector<uint8_t> indexes(256, 0);
	int count=1;
	for (auto cppair : charmap) {
		if (cppair.first > 255 || count > 255)
			break;
		indexes[cppair.first] = uint8_t(count++);
	}
	for (uint8_t c : indexes)
		writeUInt8(os, c);
}


/** Writes mapping data in subtable format 4 (segment mapping to delta values, UCS-2). */
void CmapTable::writeSubTableFormat4 (ostream &os, const RangeMap &charmap) const {
	uint16_t numSegments = charmap.numRanges()+1;
	uint16_t searchRange = 1 << (util::ilog2(numSegments)+1);
	uint16_t entrySelector = util::ilog2(searchRange/2);
	uint16_t rangeShift = 2*numSegments - searchRange;

	writeUInt16(os, 4);        // subtable format
	writeUInt16(os, 8*2+4*2*numSegments); // table length
	writeUInt16(os, 0);        // language: none
	writeUInt16(os, numSegments*2);
	writeUInt16(os, searchRange);
	writeUInt16(os, entrySelector);
	writeUInt16(os, rangeShift);
	for (size_t i=0; i < charmap.numRanges(); i++)
		writeUInt16(os, charmap.getRange(i).max());  // end character code of current segment
	writeUInt16(os, 0xFFFF);   // final end character code
	writeUInt16(os, 0);        // reserved pad
	for (size_t i=0; i < charmap.numRanges(); i++)
		writeUInt16(os, charmap.getRange(i).min());  // start character code of current segment
	writeUInt16(os, 0xFFFF);   // final start character code
	int count=1;
	for (size_t i=0; i < charmap.numRanges(); i++) {
		const auto &range = charmap.getRange(i);
		int cstart = range.min();
		writeInt16(os, count-cstart); // idDelta of current segment
		count += range.numKeys();
	}
	writeUInt16(os, 1);    // final idRange (maps 0xFFFF to 0)
	for (uint16_t i=0; i < numSegments; i++)
		writeUInt16(os, 0); // idRangeOffsets = 0 (no offsets into glyphIdArray required)
}


/** Writes mapping data in subtable format 6 (trimmed table mapping, UCS-2). */
void CmapTable::writeSubTableFormat6 (ostream &os, const RangeMap &charmap) const {
	writeUInt16(os, 6); // subtable format
	writeUInt16(os, 5*2+2*charmap.numValues()); // table length
	writeUInt16(os, 0); // language: none
	writeUInt16(os, charmap.getRange(0).min()); // first character code
	writeUInt16(os, charmap.getRange(0).numKeys());
	for (size_t i=0; i < charmap.getRange(0).numKeys(); i++)
		writeUInt16(os, i+1);
}


/** Writes mapping data in subtable format 12 (segmented coverage, UCS-4). */
void CmapTable::writeSubTableFormat12 (ostream &os, const RangeMap &charmap) const {
	writeUInt16(os, 12); // subtable format
	writeUInt16(os, 0);  // reserved
	writeUInt32(os, 2*2+3*4+charmap.numRanges()*3*4); // table length
	writeUInt32(os, 0);  // language
	writeUInt32(os, charmap.numRanges());
	uint32_t startIndex=1;
	for (size_t i=0; i < charmap.numRanges(); i++) {
		const auto &range = charmap.getRange(i);
		writeUInt32(os, range.min());  // startCharCode
		writeUInt32(os, range.max());  // endCharCode
		writeUInt32(os, startIndex);   // startGlyphID
		startIndex += range.numKeys();
	}
}
