/*************************************************************************
** GFGlyphTracer.h                                                      **
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

#ifndef DVISVGM_GFGLYPHTRACER_H
#define DVISVGM_GFGLYPHTRACER_H

#include <fstream>
#include <string>
#include "GFTracer.h"
#include "Glyph.h"

class GFGlyphTracer : public GFTracer
{
	public:
		struct Callback {
			virtual ~Callback () {}
			virtual void setFont (const std::string &fontname) {}
			virtual void beginChar (UInt8 c) {}
			virtual void endChar (UInt8 c) {}
			virtual void emptyChar (UInt8 c) {}
		};

	public:
		GFGlyphTracer ();
		GFGlyphTracer (std::string &fname, double upp, Callback *cb=0);
		void reset (std::string &fname, double upp);
		void setCallback (Callback *cb) {_callback = cb;}
		bool executeChar (UInt8 c);
		void moveTo (double x, double y);
		void lineTo (double x, double y);
		void curveTo (double c1x, double c1y, double c2x, double c2y, double x, double y);
		void closePath ();
		void endChar (UInt32 c);
		void setGlyph (Glyph &glyph)   {_glyph = &glyph;}
		const Glyph& getGlyph () const {return *_glyph;}

	private:
		std::ifstream _ifs;
		Glyph *_glyph;
		Callback *_callback;
};

#endif
