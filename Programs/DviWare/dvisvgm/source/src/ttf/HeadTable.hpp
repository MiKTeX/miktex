/*************************************************************************
** HeadTable.hpp                                                        **
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

#pragma once

#include <limits>
#include "TTFTable.hpp"

namespace ttf {

/** This class provides the functions required to write the font header table of a TTF/OTF font.
 *  https://www.microsoft.com/typography/otspec/head.htm */
class HeadTable : public TTFTable {
	friend class TTFWriter;
	public:
		uint32_t tag () const override {return name2id("head");}
		void updateGlobalBbox (int16_t xmin, int16_t ymin, int16_t xmax, int16_t ymax);
		void write (std::ostream &os) const override;
		static int offsetToChecksum () {return 2+2+4;}

	protected:
		void setLongOffsetFormat () {_indexToLocFormat = 1;}

	private:
		int16_t _xMin = std::numeric_limits<int16_t>::max();
		int16_t _yMin = std::numeric_limits<int16_t>::max();
		int16_t _xMax = std::numeric_limits<int16_t>::min();
		int16_t _yMax = std::numeric_limits<int16_t>::min();
		int16_t _indexToLocFormat = 0;  // 0: short (16-bit) format, 1: long (32-bit) format
};

} // namespace ttf
