/*************************************************************************
** GFGlyphTracer.cpp                                                    **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2022 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include "GFGlyphTracer.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;

/** Constructs a new glyph tracer.
 *  @param[in] is GF input stream
 *  @param[in] upp target units per PS point */
GFGlyphTracer::GFGlyphTracer (const string &fname, double upp, Callback *cb)
	: GFTracer(_ifs, upp), _callback(cb)
{
	if (_callback)
		_callback->setFont(fname);
#if defined(MIKTEX_WINDOWS)
        _ifs.open(EXPATH_(fname), ios::binary);
#else
	_ifs.open(fname, ios::binary);
#endif
}


void GFGlyphTracer::reset (const string &fname, double upp) {
	if (_callback)
		_callback->setFont(fname);
	if (_ifs.is_open())
		_ifs.close();
	unitsPerPoint(upp);
#if defined(MIKTEX_WINDOWS)
        _ifs.open(EXPATH_(fname), ios::binary);
#else
	_ifs.open(fname, ios::binary);
#endif
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
