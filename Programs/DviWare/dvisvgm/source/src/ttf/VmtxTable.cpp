/*************************************************************************
** VmtxTable.cpp                                                        **
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
#include "VmtxTable.hpp"
#include "TTFWriter.hpp"
#include "../Font.hpp"

using namespace std;
using namespace ttf;


void VmtxTable::write (ostream &os) const {
	auto runStart = _heightInfos.end();
	if (!_heightInfos.empty())
		runStart -= _runLengthOfAdvHeight-1;
	// write longVerMetrics up to the beginning of the repeating
	// advance heights at the end of the container
	for (auto it = _heightInfos.begin(); it != runStart; ++it) {
		writeUInt16(os, it->advHeight);
		writeInt16(os, it->tsb);
	}
	// write the trailing top side bearings
	for (; runStart != _heightInfos.end(); ++runStart)
		writeInt16(os, runStart->tsb);
}


void VmtxTable::updateHeightData (uint32_t c, int16_t ymin, int16_t ymax) {
	const PhysicalFont &font = ttfWriter()->getFont();
	double extend = font.style() ? font.style()->extend : 1;
	double scale = ttfWriter()->unitsPerEmFactor();
	int16_t h = (c == 0 ? ymax : round(scale*extend*font.vAdvance(c)));
	if (_heightInfos.empty() || h == _heightInfos.back().advHeight)
		_runLengthOfAdvHeight++;
	else
		_runLengthOfAdvHeight = 1;
	_heightInfos.emplace_back(ymin, h);
	ttfWriter()->updateGlobalAdvanceHeight(h, _heightInfos.size()-_runLengthOfAdvHeight+1);
}