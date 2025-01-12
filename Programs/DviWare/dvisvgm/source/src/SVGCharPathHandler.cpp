/*************************************************************************
** SVGCharPathHandler.cpp                                               **
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
#include <sstream>
#include "Font.hpp"
#include "FontManager.hpp"
#include "SVGCharPathHandler.hpp"
#include "utility.hpp"
#include "SVGElement.hpp"

using namespace std;

/** Constructs a new path builder.
 *  @param[in] createUseElements determines whether to create "use" elements to reference previous paths or not
 *  @param[in] relativePathCommands determines whether to create relative or absolute SVG path commands */
SVGCharPathHandler::SVGCharPathHandler (bool createUseElements, bool relativePathCommands)
	: _relativePathCommands(relativePathCommands)
{
	if (createUseElements)
		_appendChar = &SVGCharPathHandler::appendUseElement;
	else
		_appendChar = &SVGCharPathHandler::appendPathElement;
}


void SVGCharPathHandler::resetContextNode () {
	SVGCharHandler::resetContextNode();
	_groupNode = nullptr;
}


/** Appends the path representation of a single character to the context element.
 *  @param[in] c code of the character to be appended
 *  @param[in] x horizontal position of the character (in bp units)
 *  @param[in] y vertical position of the character (in bp units) */
void SVGCharPathHandler::appendChar (uint32_t c, double x, double y) {
	if (_font.changed()) {
		_fontColor.set(_font.get()->color());
		if (_fontColor.changed() && _fontColor.get() != Color::BLACK)
			_fillColor.changed(true); // ensure application of text color when resetting the font color to black
		_font.changed(false);
	}
	// Apply text color changes only if the color of the entire font is black.
	// Glyphs of non-black fonts (e.g. defined in a XeTeX document) can't change their color.
	CharProperty<Color> &color = (_fontColor.get() != Color::BLACK) ? _fontColor : _fillColor;
	bool applyColor = color.get() != Color::BLACK || (SVGElement::USE_CURRENTCOLOR && SVGElement::CURRENTCOLOR == Color::BLACK);
	bool applyMatrix = !_matrix->isIdentity();
	bool applyOpacity = !_opacity->isFillDefault();
	if (!_groupNode) {
		color.changed(applyColor);
		_matrix.changed(applyMatrix);
	}
	if (color.changed() || _matrix.changed() || _opacity.changed()) {
		resetContextNode();
		if (applyColor || applyMatrix || applyOpacity) {
			_groupNode = pushContextNode(util::make_unique<SVGElement>("g"));
			contextNode()->setFillColor(color);
			contextNode()->setFillOpacity(_opacity->fillalpha());
			contextNode()->setTransform(_matrix);
		}
		color.changed(false);
		_matrix.changed(false);
		_opacity.changed(false);
	}
	const Font *font = _font.get();
	if (font->verticalLayout()) {
		// move glyph graphics so that its origin is located at the top center position
		GlyphMetrics metrics;
		font->getGlyphMetrics(c, _vertical, metrics);
		x -= metrics.wl;
		if (_vertical) {
			auto physicalFont = font_cast<const PhysicalFont *>(font);
			if (!physicalFont)
				y += metrics.d;
			else {
				// Center glyph between top and bottom border of the TFM box.
				// This is just an approximation used until I find a way to compute
				// the exact location in vertical mode.
				GlyphMetrics exact_metrics;
				physicalFont->getExactGlyphBox(c, exact_metrics, false, nullptr);
				double ed = max(0.0, exact_metrics.d);
				double eh = max(0.0, exact_metrics.h);
				y += eh + (metrics.d - eh - ed) / 2;
			}
		}
	}
	Matrix rotation(1);
	if (_vertical && !font->verticalLayout()) {
		// alphabetic text designed for horizontal mode
		// must be rotated by 90 degrees if in vertical mode
		rotation.translate(-x, -y);
		rotation.rotate(90);
		rotation.translate(x, y);
	}
	(this->*_appendChar)(c, x, y, rotation);
}


void SVGCharPathHandler::appendUseElement (uint32_t c, double x, double y, const Matrix &matrix) {
	string id = "#g" + to_string(FontManager::instance().fontID(_font)) + "-" + to_string(c);
	auto useNode = util::make_unique<SVGElement>("use");
	useNode->addAttribute("x", x);
	useNode->addAttribute("y", y);
	useNode->addAttribute("xlink:href", id);
	useNode->setFillOpacity(_opacity->blendMode()); // add blend mode style here because it's not inheritable
	useNode->setTransform(matrix);
	contextNode()->append(std::move(useNode));
}


void SVGCharPathHandler::appendPathElement (uint32_t c, double x, double y, const Matrix &matrix) {
	Glyph glyph;
	auto pf = font_cast<const PhysicalFont*>(_font.get());
	if (pf && pf->getGlyph(c, glyph, nullptr)) {
		double sx = pf->scaledSize()/pf->unitsPerEm();
		double sy = -sx;
		ostringstream oss;
		glyph.writeSVG(oss, _relativePathCommands, sx, sy, x, y);
		auto glyphNode = util::make_unique<SVGElement>("path");
		glyphNode->addAttribute("d", oss.str());
		glyphNode->setTransform(matrix);
		contextNode()->append(std::move(glyphNode));
	}
}
