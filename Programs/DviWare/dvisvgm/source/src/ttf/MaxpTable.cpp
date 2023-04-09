/*************************************************************************
** MaxpTable.cpp                                                        **
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
#include "MaxpTable.hpp"
#include "TTFWriter.hpp"

using namespace std;
using namespace ttf;

void MaxpTable::write (ostream &os) const {
	writeUInt32(os, 0x10000);  // version 1.0, required for TTF fonts
	writeUInt16(os, ttfWriter()->getUnicodeCharMap().numValues()+1);  // number of glyphs including .notdef
	writeUInt16(os, _maxPoints);
	writeUInt16(os, _maxContours);
	writeUInt16(os, 0);  // maxCompositePoints
	writeUInt16(os, 0);  // maxCompositeContours
	writeUInt16(os, 1);  // maxZones (1 = don't use twilight zone, 2 otherwise)
	writeUInt16(os, 0);  // maxTwilightPoints
	writeUInt16(os, 0);  // maxStorage
	writeUInt16(os, 0);  // maxFunctionDefs
	writeUInt16(os, 0);  // maxInstructionDefs
	writeUInt16(os, 0);  // maxStackElements
	writeUInt16(os, 0);  // maxSizeOfInstructions
	writeUInt16(os, 0);  // maxComponentElements
	writeUInt16(os, 0);  // maxComponentDepth
}


void MaxpTable::updateContourInfo (uint16_t maxPoints, uint16_t maxContours) {
	_maxPoints = max(_maxPoints, maxPoints);
	_maxContours = max(_maxContours, maxContours);
}