/*************************************************************************
** PostTable.cpp                                                        **
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
#include "PostTable.hpp"
#include "TTFWriter.hpp"
#include "../Font.hpp"

using namespace std;
using namespace ttf;

#if 0
/** Writes a string in Pascal format to an output stream. The string should not be
 *  longer than 255 characters. Otherwise, it's truncated accordingly. */
static void write_pascal_string (ostream &os, const string &str) {
	size_t len = std::min(str.length(), size_t(255));
	writeUInt8(os, len);   // first byte determines the string length
	for (size_t i=0; i < len; i++)
		writeUInt8(os, str[i]);
}
#endif


void PostTable::write (ostream &os) const {
	writeUInt32(os, 0x30000);   // table version 3
	writeInt32(os, 0);   // italic angle
	writeInt16(os, 0);   // underline position
	writeInt16(os, 0);   // underline thickness
	writeUInt32(os, ttfWriter()->isFixedPitch());  // 1 = fixed pitch
	writeUInt32(os, 0);  // minMemType42
	writeUInt32(os, 0);  // maxMemType42
	writeUInt32(os, 0);  // minMemType1
	writeUInt32(os, 0);  // maxMemType1
/*	writeUInt16(os, ttfWriter()->getUnicodeCharMap().numValues());
	// write name indexes in glyph ID order (index 0 = ".notdef")
	uint16_t index=0;
	for (auto cp2charcode : ttfWriter()->getUnicodeCharMap()) {
		string name = ttfWriter()->getFont().glyphName(cp2charcode.second);
		writeUInt16(os, name.empty() || name == ".notdef" ? 0 : ++index);
	}
	// write the glyph names in glyph ID order
	for (auto cp2charcode : ttfWriter()->getUnicodeCharMap()) {
		string name = ttfWriter()->getFont().glyphName(cp2charcode.second);
		if (!name.empty() && name != ".notdef")
			write_pascal_string(os, name);
	}*/
}
