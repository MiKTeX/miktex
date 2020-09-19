/*************************************************************************
** GFTracer.cpp                                                         **
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

#include <fstream>
#include <potracelib.h>
#include "GFTracer.hpp"
#include "Glyph.hpp"
#include "Message.hpp"

using namespace std;


/** Constructs a new GFTracer.
 *  @param[in] is GF file is read from this stream
 *  @param[in] upp target units per PS point */
GFTracer::GFTracer (istream &is, double upp)
	: GFReader(is), _unitsPerPoint(upp)
{
}


void GFTracer::beginChar (uint32_t c) {
}


void GFTracer::endChar (uint32_t c) {
	const Bitmap &bitmap = getBitmap();
	if (bitmap.empty())
		return;

	// prepare potrace's bitmap structure
	vector<potrace_word> buffer;
	potrace_bitmap_t pobitmap;
	pobitmap.w = bitmap.width();
	pobitmap.h = bitmap.height();
	pobitmap.dy = bitmap.copy(buffer);
	pobitmap.map = &buffer[0];
	potrace_param_t *param = potrace_param_default();
	potrace_state_t *state = potrace_trace(param, &pobitmap);
	potrace_param_free(param);

	if (!state || state->status == POTRACE_STATUS_INCOMPLETE)
		Message::wstream(true) << "error while tracing character\n";
	else {
		double hsf=1.0, vsf=1.0; // horizontal a d vertical scale factor
		if (_unitsPerPoint != 0.0) {
			hsf = _unitsPerPoint/getHPixelsPerPoint();  // horizontal scale factor
			vsf = _unitsPerPoint/getVPixelsPerPoint();  // vertical scale factor
		}
		for (potrace_path_t *path = state->plist; path; path = path->next) {
			potrace_dpoint_t &p = path->curve.c[path->curve.n-1][2]; // start/end point
			moveTo(hsf*(p.x+bitmap.xshift()), vsf*(p.y+bitmap.yshift()));
			for (int i=0; i < path->curve.n; i++) {
				if (path->curve.tag[i] == POTRACE_CURVETO) {
					curveTo(hsf*(path->curve.c[i][0].x+bitmap.xshift()), vsf*(path->curve.c[i][0].y+bitmap.yshift()),
							  hsf*(path->curve.c[i][1].x+bitmap.xshift()), vsf*(path->curve.c[i][1].y+bitmap.yshift()),
							  hsf*(path->curve.c[i][2].x+bitmap.xshift()), vsf*(path->curve.c[i][2].y+bitmap.yshift()));
				}
				else {
					lineTo(hsf*(path->curve.c[i][1].x+bitmap.xshift()), vsf*(path->curve.c[i][1].y+bitmap.yshift()));
					if (i == path->curve.n-1)
						closePath();
					else
						lineTo(hsf*(path->curve.c[i][2].x+bitmap.xshift()), vsf*(path->curve.c[i][2].y+bitmap.yshift()));
				}
			}
		}
	}
	potrace_state_free(state);
}
