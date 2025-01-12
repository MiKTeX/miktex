/*************************************************************************
** HmtxTable.cpp                                                        **
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
#include <cmath>
#include "HmtxTable.hpp"
#include "TTFWriter.hpp"
#include "../Font.hpp"

using namespace std;
using namespace ttf;

void HmtxTable::write (ostream &os) const {
	auto runStart = _widthInfos.end();
	if (!_widthInfos.empty())
		runStart -= _runLengthOfAdvWidth-1;
	// write longHorMetrics up to the beginning of the repeating
	// advance widths at the end of the container
	for (auto it = _widthInfos.begin(); it != runStart; ++it) {
		writeUInt16(os, it->advWidth);
		writeInt16(os, it->lsb);
	}
	// write the trailing left side bearings
	for (; runStart != _widthInfos.end(); ++runStart)
		writeInt16(os, runStart->lsb);
}


void HmtxTable::updateWidthData (uint32_t c, int16_t xmin, int16_t xmax) {
	const PhysicalFont &font = ttfWriter()->getFont();
	double extend = font.style() ? font.style()->extend : 1;
	double scale = ttfWriter()->unitsPerEmFactor();
	int16_t w = (c == 0 ? xmax : int16_t(round(scale*extend*font.hAdvance(c))));
	if (_isFixedPitch && c > 0 && w != _width) {
		if (_width == 0)
			_width = w;
		else
			_isFixedPitch = false;
	}
	if (_widthInfos.empty() || w == _widthInfos.back().advWidth)
		_runLengthOfAdvWidth++;
	else
		_runLengthOfAdvWidth = 1;
	_widthInfos.emplace_back(xmin, w);
	ttfWriter()->updateGlobalAdvanceWidth(w, _widthInfos.size()-_runLengthOfAdvWidth+1);
}