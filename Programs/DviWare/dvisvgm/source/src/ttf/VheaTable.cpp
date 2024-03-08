/*************************************************************************
** VheaTable.cpp                                                        **
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
#include "VheaTable.hpp"
#include "TTFWriter.hpp"
#include "../Font.hpp"

using namespace std;
using namespace ttf;


void VheaTable::write (ostream &os) const {
	const PhysicalFont &font = ttfWriter()->getFont();
	double scale = ttfWriter()->unitsPerEmFactor();
	writeUInt32(os, 0x10000);  // table version 1.0
	writeInt16(os, round(font.ascent()*scale));
	writeInt16(os, round(-font.descent()*scale));
	writeInt16(os, 0);   // line gap, always 0
	writeUInt16(os, _advanceHeightMax);
	writeInt16(os, _minTopSideBearing);
	writeInt16(os, _minBottomSideBearing);
	writeInt16(os, _yMaxExtent);
	writeInt16(os, 1);   // caretSlopeRise (1 = vertical)
	writeInt16(os, 0);   // caretSlopeRun (0 = vertical)
	writeInt16(os, 0);   // caretOffset
	writeInt16(os, 0);   // reserved
	writeInt16(os, 0);   // reserved
	writeInt16(os, 0);   // reserved
	writeInt16(os, 0);   // reserved
	writeInt16(os, 0);   // metricDataFormat (always 0)
	writeUInt16(os, _numberOfVMetrics);
}


void VheaTable::updateYMinMax (uint32_t c, int16_t ymin, int16_t ymax) {
	const PhysicalFont &font = ttfWriter()->getFont();
	double extend = font.style() ? font.style()->extend : 1;
	double scale = ttfWriter()->unitsPerEmFactor();
	int16_t advance = (c == 0 ? ymax : round(scale*extend*font.vAdvance(c)));
	_yMaxExtent = max(_yMaxExtent, ymax);
	_minTopSideBearing = min(_minTopSideBearing, ymin);
	_minBottomSideBearing = min(_minBottomSideBearing, int16_t(advance-ymax));
}


void VheaTable::updateAdvanceHeight (uint16_t h, uint16_t numberOfVMetrics) {
	_advanceHeightMax = max(_advanceHeightMax, h);
	_numberOfVMetrics = numberOfVMetrics;
}
