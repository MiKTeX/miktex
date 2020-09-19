/*************************************************************************
** SVGSingleCharTextHandler.cpp                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include "SVGSingleCharTextHandler.hpp"
#include "XMLNode.hpp"

using namespace std;

void SVGSingleCharTextHandler::appendChar (uint32_t c, double x, double y) {
	const Font *font = _font.get();
	auto textNode = createTextNode(x, y);
	textNode->append(XMLString(font->unicode(c), false));
	// Apply color changes only if the color differs from black and if the font color itself is black.
	// Glyphs from non-black fonts (e.g. defined in a XeTeX document) can't change their color.
	if (_color.get() != Color::BLACK && font->color() == Color::BLACK) {
		textNode->addAttribute("fill", _color.get().svgColorString());
		_color.changed(false);
	}
	contextNode()->append(std::move(textNode));
}
