/*************************************************************************
** HeadTable.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <cmath>
#include <chrono>
#include <ctime>
#include <utility>
#include "HeadTable.hpp"
#include "TTFWriter.hpp"

using namespace std;
using namespace ttf;


void HeadTable::updateGlobalBbox (int16_t xmin, int16_t ymin, int16_t xmax, int16_t ymax) {
	if (xmin < _xMin) _xMin = xmin;
	if (ymin < _yMin) _yMin = ymin;
	if (xmax > _xMax) _xMax = xmax;
	if (ymax > _yMax) _yMax = ymax;
}


/** Writes the head table to a given output stream.
 *  https://docs.microsoft.com/en-us/typography/opentype/spec/head */
void HeadTable::write (ostream &os) const {
	writeUInt16(os, 1);       // major version
	writeUInt16(os, 0);       // minor version
	writeUInt32(os, 0x10000); // font revision (1.0)
	writeUInt32(os, 0);       // global checksum, computed and set by TTFWriter
	writeUInt32(os, 0x5F0F3CF5);  // magic number
	writeUInt16(os, 1+2);         // flags (baseline at y=0, left sidebearing point at x=0)
	writeUInt16(os, uint16_t(std::round(ttfWriter()->targetUnitsPerEm())));
	writeUInt32(os, 0);  // creation time, upper dword
	writeUInt32(os, 0);  // creation time, lower dword
	writeUInt32(os, 0);  // modification time, upper dword
	writeUInt32(os, 0);  // modification time, lower dword
	writeInt16(os, _xMin);
	writeInt16(os, _yMin);
	writeInt16(os, _xMax);
	writeInt16(os, _yMax);
	writeUInt16(os, 0); // macStyle
	writeUInt16(os, 8); // smallest readable size in pixels
	writeInt16(os, 2);  // fontDirectionHint (deprecated, always 2)
	writeInt16(os, _indexToLocFormat);
	writeInt16(os, 0);  // glyphDataFormat
}
