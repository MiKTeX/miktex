/*************************************************************************
** SVGCharTspanTextHandler.hpp                                          **
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

#ifndef SVGCHARTSPANTEXTHANDLER_HPP
#define SVGCHARTSPANTEXTHANDLER_HPP

#include "SVGCharHandler.hpp"

class SVGCharTspanTextHandler : public SVGCharTextHandler {
	public:
		explicit SVGCharTspanTextHandler (bool selectFontByClass);
		void notifyXAdjusted () override {_xchanged = true;}
		void notifyYAdjusted()  override {_ychanged = true;}
		void appendChar (uint32_t c, double x, double y) override;
		void setInitialContextNode (SVGElement *node) override;

	protected:
		void resetContextNode () override;

	private:
		bool _xchanged, _ychanged;
		SVGElement *_textNode;
		SVGElement *_tspanNode;
};

#endif
