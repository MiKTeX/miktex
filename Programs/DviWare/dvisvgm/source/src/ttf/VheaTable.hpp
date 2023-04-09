/*************************************************************************
** VheaTable.hpp                                                        **
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

#pragma once
#include <limits>
#include "TTFTable.hpp"

namespace ttf {

/** This class provides the functions required to write the vertical header table of a TTF/OTF font.
 *  https://www.microsoft.com/typography/otspec/vhea.htm */
class VheaTable : public TTFTable {
	friend class TTFWriter;
	public:
		uint32_t tag () const override {return name2id("vhea");}
		void write (std::ostream &os) const override;

	protected:
		void updateYMinMax (uint32_t c, int16_t ymin, int16_t ymax);
		void updateAdvanceHeight (uint16_t h, uint16_t numberOfVMetrics);

	private:
		uint16_t _advanceHeightMax = 0;
		int16_t _minTopSideBearing = std::numeric_limits<int16_t>::max();
		int16_t _minBottomSideBearing = std::numeric_limits<int16_t>::max();
		int16_t _yMaxExtent = std::numeric_limits<int16_t>::min();
		uint16_t _numberOfVMetrics = 0;
};

} // namespace ttf
