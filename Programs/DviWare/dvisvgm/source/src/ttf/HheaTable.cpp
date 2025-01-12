/*************************************************************************
** HheaTable.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include "HheaTable.hpp"
#include "TTFWriter.hpp"
#include "../Font.hpp"

using namespace std;
using namespace ttf;


void HheaTable::write (ostream &os) const {
	const PhysicalFont &font = ttfWriter()->getFont();
	double scale = ttfWriter()->unitsPerEmFactor();
	writeUInt16(os, 1);  // major version
	writeUInt16(os, 0);  // minor version
	writeInt16(os, round(font.ascent()*scale));
	writeInt16(os, round(-font.descent()*scale));
	writeInt16(os, 0);   // line gap
	writeUInt16(os, _advanceWidthMax);
	writeInt16(os, _minLeftSideBearing);
	writeInt16(os, _minRightSideBearing);
	writeInt16(os, _xMaxExtent);
	writeInt16(os, 1);   // caretSlopeRise (1 = vertical)
	writeInt16(os, 0);   // caretSlopeRun (0 = vertical)
	writeInt16(os, 0);   // caretOffset
	writeInt16(os, 0);   // reserved
	writeInt16(os, 0);   // reserved
	writeInt16(os, 0);   // reserved
	writeInt16(os, 0);   // reserved
	writeInt16(os, 0);   // metricDataFormat (always 0)
	writeUInt16(os, _numberOfHMetrics);
}


void HheaTable::updateXMinMax (uint32_t c, int16_t xmin, int16_t xmax) {
	const PhysicalFont &font = ttfWriter()->getFont();
	double extend = font.style() ? font.style()->extend : 1;
	double scale = ttfWriter()->unitsPerEmFactor();
	int16_t advance = (c == 0 ? xmax : round(scale*extend*font.hAdvance(c)));
	_xMaxExtent = max(_xMaxExtent, xmax);
	_minLeftSideBearing = min(_minLeftSideBearing, xmin);
	_minRightSideBearing = min(_minRightSideBearing, int16_t(advance-xmax));
}


void HheaTable::updateAdvanceWidth (uint16_t w, uint16_t numberOfHMetrics) {
	_advanceWidthMax = max(_advanceWidthMax, w);
	_numberOfHMetrics = numberOfHMetrics;
}
