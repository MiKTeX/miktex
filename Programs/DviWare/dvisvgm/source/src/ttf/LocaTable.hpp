/*************************************************************************
** LocaTable.hpp                                                        **
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

#include <vector>
#include "TTFTable.hpp"

namespace ttf {

/** This class provides the functions required to write the loca table of a TTF/OTF font.
 *  https://www.microsoft.com/typography/otspec/loca.htm */
class LocaTable : public TTFTable {
	friend class TTFWriter;
	public:
		uint32_t tag () const override {return name2id("loca");}

		void write (std::ostream &os) const override {
			for (uint32_t offs : _offsets) {
				if (_isShortFormat)
					writeUInt16(os, offs/2);
				else
					writeUInt32(os, offs);
			}
		}

	protected:
		void addOffset (uint32_t offset) {
			_offsets.push_back(offset);
			if (offset/2 > 0xFFFF)
				_isShortFormat = false;
		}

	private:
		bool _isShortFormat=true;
		std::vector<uint32_t> _offsets;
};

} // namespace ttf
