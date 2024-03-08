/*************************************************************************
** HmtxTable.hpp                                                        **
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

#include <vector>
#include "TTFTable.hpp"

namespace ttf {

/** This class provides the functions required to write the horizontal metrics table of a TTF/OTF font.
 *  https://www.microsoft.com/typography/otspec/hmtx.htm */
class HmtxTable : public TTFTable {
	friend TTFWriter;
	struct CharWidthInfo {
		CharWidthInfo (int16_t l, int16_t w) : lsb(l), advWidth(w) {}
		uint16_t lsb;       // left side bearing
		uint16_t advWidth;  // unscaled advance width
	};
	public:
		uint32_t tag () const override {return name2id("hmtx");}
		void write (std::ostream &os) const override;
		bool isFixedPitch () const {return _isFixedPitch;}

	protected:
		void updateWidthData (uint32_t c, int16_t xmin, int16_t xmax);

	private:
		std::vector<CharWidthInfo> _widthInfos;
		size_t _runLengthOfAdvWidth=0;  // number of identical advWidth values at end of _widthInfos
		bool _isFixedPitch=true;
		int16_t _width=0;
};

} // namespace ttf
