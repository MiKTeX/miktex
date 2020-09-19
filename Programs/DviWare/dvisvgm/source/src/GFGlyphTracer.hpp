/*************************************************************************
** GFGlyphTracer.hpp                                                    **
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

#ifndef GFGLYPHTRACER_HPP
#define GFGLYPHTRACER_HPP

#include <fstream>
#include <string>
#include "GFTracer.hpp"
#include "Glyph.hpp"

class GFGlyphTracer : public GFTracer {
	public:
		struct Callback {
			virtual ~Callback () =default;
			virtual void setFont (const std::string &fontname) {}
			virtual void beginChar (uint8_t c) {}
			virtual void endChar (uint8_t c) {}
			virtual void emptyChar (uint8_t c) {}
		};

	public:
		GFGlyphTracer () : GFTracer(_ifs, 0) {}
		GFGlyphTracer (std::string &fname, double upp, Callback *cb=nullptr);
		void reset (std::string &fname, double upp);
		void setCallback (Callback *cb) {_callback = cb;}
		bool executeChar (uint8_t c) override;
		void moveTo (double x, double y) override;
		void lineTo (double x, double y) override;
		void curveTo (double c1x, double c1y, double c2x, double c2y, double x, double y) override;
		void closePath () override;
		void endChar (uint32_t c) override;
		void setGlyph (Glyph &glyph)   {_glyph = &glyph;}
		const Glyph& getGlyph () const {return *_glyph;}

	private:
		std::ifstream _ifs;
		Glyph *_glyph = nullptr;
		Callback *_callback = nullptr;
};

#endif
