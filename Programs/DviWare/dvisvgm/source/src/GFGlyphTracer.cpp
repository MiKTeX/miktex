/*************************************************************************
** GFGlyphTracer.cpp                                                    **
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

#include <config.h>
#include "GFGlyphTracer.hpp"

using namespace std;

GFGlyphTracer::GFGlyphTracer () : GFTracer(_ifs, 0), _glyph(0), _callback(0)
{
}

/** Constructs a new glyph tracer.
 *  @param[in] is GF input stream
 *  @param[in] upp target units per PS point */
GFGlyphTracer::GFGlyphTracer (string &fname, double upp, Callback *cb)
	: GFTracer(_ifs, upp), _glyph(0), _callback(cb)
{
	if (_callback)
		_callback->setFont(fname);
	_ifs.open(fname.c_str(), ios::binary);
}


void GFGlyphTracer::reset (string &fname, double upp) {
	if (_callback)
		_callback->setFont(fname);
	if (_ifs.is_open())
		_ifs.close();
	unitsPerPoint(upp);
	_ifs.open(fname.c_str(), ios::binary);
}


bool GFGlyphTracer::executeChar (uint8_t c) {
	if (!_glyph)
		return false;

	if (_callback)
		_callback->beginChar(c);
	bool ok = GFTracer::executeChar(c);
	if (_callback) {
		if (ok)
			_callback->endChar(c);
		else
			_callback->emptyChar(c);
	}
	return ok;
}


void GFGlyphTracer::moveTo (double x, double y) {
	_glyph->moveto(int(x), int(y));
}


void GFGlyphTracer::lineTo (double x, double y) {
	_glyph->lineto(int(x), int(y));
}


void GFGlyphTracer::curveTo (double c1x, double c1y, double c2x, double c2y, double x, double y) {
	_glyph->cubicto(int(c1x), int(c1y), int(c2x), int(c2y), int(x), int(y));
}


void GFGlyphTracer::closePath () {
	_glyph->closepath();
}


void GFGlyphTracer::endChar (uint32_t c) {
	_glyph->clear();
	GFTracer::endChar(c);
}
