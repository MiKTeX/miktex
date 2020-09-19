/*************************************************************************
** SVGCharTspanTextHandler.cpp                                          **
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

#include "SVGCharTspanTextHandler.hpp"
#include "utility.hpp"
#include "XMLNode.hpp"

using namespace std;

SVGCharTspanTextHandler::SVGCharTspanTextHandler (bool selectFontByClass)
	: SVGCharTextHandler(selectFontByClass)
{
	resetContextNode();
}


void SVGCharTspanTextHandler::appendChar (uint32_t c, double x, double y) {
	// changes of fonts and transformations require a new text element
	if (!_textNode || _font.changed() || _matrix.changed() || _vertical.changed()) {
		resetContextNode();
		_textNode = pushContextNode(createTextNode(x, y));
		_color.changed(true);  // force creating tspan with color attribute if current color differs from font color
	}
	if (_tspanNode && (_xchanged || _ychanged || _color.changed())) {
		// if drawing position or color was explicitly changed, finish current tspan element
		popContextNode();
		_tspanNode = nullptr;
	}
	// Apply text color changes only if the color of the entire font is black.
	// Glyphs of non-black fonts (e.g. defined in a XeTeX document) can't change their color.
	bool applyColor = _color.get() != Color::BLACK && _font.get()->color() == Color::BLACK;
	if (_xchanged || _ychanged || (_color.changed() && applyColor)) {
		_tspanNode = pushContextNode(util::make_unique<XMLElement>("tspan"));
		if (applyColor)
			_tspanNode->addAttribute("fill", _color.get().svgColorString());
		_color.changed(false);
		if (_xchanged) {
			if (_vertical) {
				// align glyphs designed for horizontal layout properly
				if (auto pf = dynamic_cast<const PhysicalFont*>(_font.get()))
					if (!pf->getMetrics()->verticalLayout())
						x += pf->scaledAscent()/2.5; // move vertical baseline to the right by strikethrough offset
			}
			_tspanNode->addAttribute("x", x);
			_xchanged = false;
		}
		if (_ychanged) {
			_tspanNode->addAttribute("y", y);
			_ychanged = false;
		}
	}
	contextNode()->append(XMLString(_font.get()->unicode(c), false));
}


void SVGCharTspanTextHandler::setInitialContextNode (XMLElement *node) {
	SVGCharHandler::setInitialContextNode(node);
	_textNode = _tspanNode = nullptr;
	_xchanged = _ychanged = false;
}


void SVGCharTspanTextHandler::resetContextNode () {
	SVGCharHandler::resetContextNode();
	_textNode = _tspanNode = nullptr;
	_xchanged = _ychanged = false;
	_font.changed(false);
	_matrix.changed(false);
	_vertical.changed(false);
}
