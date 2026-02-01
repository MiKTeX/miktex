/*************************************************************************
** Base14Fonts.cpp                                                      **
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

#include "../algorithm.hpp"
#include "Base14Fonts.hpp"

using namespace std;

extern const MemoryFontData Dingbats_cff;
extern const MemoryFontData NimbusMonoPS_Bold_cff;
extern const MemoryFontData NimbusMonoPS_BoldItalic_cff;
extern const MemoryFontData NimbusMonoPS_Italic_cff;
extern const MemoryFontData NimbusMonoPS_Regular_cff;
extern const MemoryFontData NimbusRoman_Bold_cff;
extern const MemoryFontData NimbusRoman_BoldItalic_cff;
extern const MemoryFontData NimbusRoman_Italic_cff;
extern const MemoryFontData NimbusRoman_Regular_cff;
extern const MemoryFontData NimbusSans_Bold_cff;
extern const MemoryFontData NimbusSans_BoldItalic_cff;
extern const MemoryFontData NimbusSans_Italic_cff;
extern const MemoryFontData NimbusSans_Regular_cff;
extern const MemoryFontData StandardSymbolsPS_cff;

/** Returns the CFF font data for a given name of a Base 14 font.
 *  If 'fontname' doesn't specify one of the 14 PDF base fonts,
 *  the function returns nullptr. */
const MemoryFontData* find_base14_font (const string &fontname) {
	struct FontData {
		const char *name;
		const MemoryFontData *font;
	} names[14] = {
		{"Courier", &NimbusMonoPS_Regular_cff},
		{"Courier-Oblique", &NimbusMonoPS_Italic_cff},
		{"Courier-Bold", &NimbusMonoPS_Bold_cff},
		{"Courier-BoldOblique", &NimbusMonoPS_BoldItalic_cff},
		{"Helvetica", &NimbusSans_Regular_cff},
		{"Helvetica-Oblique", &NimbusSans_Italic_cff},
		{"Helvetica-Bold", &NimbusSans_Bold_cff},
		{"Helvetica-BoldOblique", &NimbusSans_BoldItalic_cff},
		{"Times-Roman", &NimbusRoman_Regular_cff},
		{"Times-Italic", &NimbusRoman_Italic_cff},
		{"Times-Bold", &NimbusRoman_Bold_cff},
		{"Times-BoldItalic", &NimbusRoman_BoldItalic_cff},
		{"Symbol", &StandardSymbolsPS_cff},
		{"ZapfDingbats", &Dingbats_cff}
	};
	auto it = algo::find_if(names, [&fontname](const FontData &data) {
		return data.name == fontname;
	});
	return it != end(names) ? it->font : nullptr;
}
