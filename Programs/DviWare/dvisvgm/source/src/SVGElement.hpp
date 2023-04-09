/*************************************************************************
** SVGElement.hpp                                                       **
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

#ifndef SVGELEMENT_HPP
#define SVGELEMENT_HPP

#include "XMLNode.hpp"

class Color;
class Matrix;
class Opacity;

class SVGElement : public XMLElement {
	public:
		enum FillRule {FR_EVENODD, FR_NONZERO};
		enum LineCap {LC_BUTT, LC_ROUND, LC_SQUARE};
		enum LineJoin {LJ_BEVEL, LJ_MITER, LJ_ROUND};

	public:
		explicit SVGElement (std::string name) : XMLElement(std::move(name)) {}
		explicit SVGElement (const XMLElement &node) : XMLElement(node) {}
		explicit SVGElement (XMLElement &&node) noexcept : XMLElement(std::move(node)) {}
		void setClipPathUrl (const std::string &url);
		void setClipRule (FillRule rule);
		void setFillColor (Color color, bool skipBlack=true);
		void setFillOpacity (const Opacity &opacity);
		void setFillOpacity (const OpacityAlpha &alpha);
		void setFillOpacity (Opacity::BlendMode blendMode);
		void setFillPatternUrl (const std::string &url);
		void setFillRule (FillRule rule);
		void setMaskUrl (const std::string &url);
		void setNoFillColor ();
		void setOpacity (const OpacityAlpha &alpha);
		void setPoints (const std::vector<DPair> &points);
		void setStrokeColor (Color color);
		void setStrokeDash (const std::vector<double> &pattern, double offset=0);
		void setStrokeDash (const std::string &pattern, double offset=0);
		void setStrokeLineCap (LineCap cap);
		void setStrokeLineJoin (LineJoin join);
		void setStrokeOpacity (const Opacity &opacity);
		void setStrokeWidth (double width);
		void setStrokeMiterLimit (double limit);
		void setTransform (const Matrix &matrix);
};

#endif
