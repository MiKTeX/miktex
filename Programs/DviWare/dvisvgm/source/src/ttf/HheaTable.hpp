/*************************************************************************
** HheaTable.hpp                                                        **
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

#pragma once
#include <limits>
#include "TTFTable.hpp"

namespace ttf {

/** This class provides the functions required to write the horizontal header table of a TTF/OTF font.
 *  https://www.microsoft.com/typography/otspec/hhea.htm */
class HheaTable : public TTFTable {
	friend class TTFWriter;
	public:
		uint32_t tag () const override {return name2id("hhea");}
		void write (std::ostream &os) const override;

	protected:
		void updateXMinMax (uint32_t c, int16_t xmin, int16_t xmax);
		void updateAdvanceWidth (uint16_t w, uint16_t numberOfHMetrics);

	private:
		uint16_t _advanceWidthMax = 0;
		int16_t _minLeftSideBearing = std::numeric_limits<int16_t>::max();
		int16_t _minRightSideBearing = std::numeric_limits<int16_t>::max();
		int16_t _xMaxExtent = std::numeric_limits<int16_t>::min();
		uint16_t _numberOfHMetrics = 0;
};

} // namespace ttf
