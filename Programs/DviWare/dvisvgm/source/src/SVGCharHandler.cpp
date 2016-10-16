/*************************************************************************
** SVGCharHandler.cpp                                                   **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2016 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include "SVGCharHandler.hpp"
#include "XMLNode.hpp"

using namespace std;


void SVGCharHandler::setInitialContextNode (XMLElementNode *node) {
	resetContextNode();
	_initialContextNode = node;
}


/** Changes the context element. All following nodes will be appended to this node. */
void SVGCharHandler::pushContextNode (XMLElementNode *node) {
	if (node && (_contextNodeStack.empty() || node != _contextNodeStack.top())) {
		contextNode()->append(node);
		_contextNodeStack.push(node);
	}
}


void SVGCharHandler::popContextNode () {
	if (!_contextNodeStack.empty())
		_contextNodeStack.pop();
}


/** Resets the context node to the initial element node. */
void SVGCharHandler::resetContextNode () {
	while (!_contextNodeStack.empty())
		_contextNodeStack.pop();
}


/** Creates and returns a new SVG text element.
 *  @param[in] x current x coordinate
 *  @param[in] y current y coordinate */
XMLElementNode* SVGCharTextHandler::createTextNode (double x, double y) const {
	const Font *font = _font.get();
	if (!font)
		return 0;
	XMLElementNode *textNode = new XMLElementNode("text");
	if (_selectFontByClass)
		textNode->addAttribute("class", string("f")+XMLString(_fontnum));
	else {

		textNode->addAttribute("font-family", font->name());
		textNode->addAttribute("font-size", XMLString(font->scaledSize()));
		if (font->color() != Color::BLACK)
			textNode->addAttribute("fill", font->color().svgColorString());
	}
	if (_vertical) {
		textNode->addAttribute("writing-mode", "tb");
		// align glyphs designed for horizontal layout properly
		if (const PhysicalFont *pf = dynamic_cast<const PhysicalFont*>(font))
			if (!pf->getMetrics()->verticalLayout()) { // alphabetic text designed for horizontal layout?
				x += pf->scaledAscent()/2.5; // move vertical baseline to the right by strikethrough offset
				textNode->addAttribute("glyph-orientation-vertical", 90); // ensure rotation
			}
	}
	textNode->addAttribute("x", x);
	textNode->addAttribute("y", y);
	if (!_matrix.get().isIdentity())
		textNode->addAttribute("transform", _matrix.get().getSVG());
	return textNode;
}