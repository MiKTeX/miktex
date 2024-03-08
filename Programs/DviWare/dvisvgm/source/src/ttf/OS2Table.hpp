/*************************************************************************
** OS2Table.hpp                                                         **
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

#include <array>
#include "TTFTable.hpp"

namespace ttf {

/** This class provides the functions required to write the OS/2 and Windows metrics
 *  table of a TTF/OTF font.
 *  https://www.microsoft.com/typography/otspec/os2.htm */
class OS2Table : public TTFTable {
	friend class TTFWriter;
	public:
		uint32_t tag () const override {return name2id("OS/2");}
		void write (std::ostream &os) const override;

	protected:
		int16_t averageCharWidth () const;
		void updateXMinMax (int16_t xmin, int16_t xmax) {_xmax = xmax;}
		void updateYMinMax (int16_t ymin, int16_t ymax) {}

	private:
		int16_t _xmax=0;
};

} // namespace ttf