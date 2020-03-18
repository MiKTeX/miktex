/*************************************************************************
** Bitmap.cpp                                                           **
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

#include <algorithm>
#include <cstdlib>
#include <limits>
#include "Bitmap.hpp"

using namespace std;


/** Constructs a Bitmap */
Bitmap::Bitmap (int minx, int maxx, int miny , int maxy) {
	resize(minx, maxx, miny, maxy);
}


/** Resizes the bitmap and clears all pixels.
 *  @param[in] minx index of leftmost pixel column
 *  @param[in] maxx index of rightmost pixel column
 *  @param[in] miny index of bottom row
 *  @param[in] maxy index of top row */
void Bitmap::resize (int minx, int maxx, int miny , int maxy) {
	_rows = abs(maxy-miny)+1;
	_cols = abs(maxx-minx)+1;
	_xshift = minx;
	_yshift = miny;
	_bpr  = _cols/8 + (_cols % 8 ? 1 : 0);  // bytes per row
	_bytes.resize(_rows*_bpr);
	std::fill(_bytes.begin(), _bytes.end(), 0);
}


/** Sets n pixels of row r to 1 starting at pixel c.
 *  @param[in] row number of row
 *  @param[in] col number of column (pixel)
 *  @param[in] n number of bits to be set */
void Bitmap::setBits (int row, int col, int n) {
	row -= _yshift;
	col -= _xshift;
	uint8_t *byte = &_bytes[row*_bpr + col/8];
	if (byte < &_bytes[0])
		return;
	const uint8_t *maxptr = &_bytes[0]+_bytes.size()-1;
	while (n > 0 && byte <= maxptr) {
		int b = 7 - col%8;          // number of leftmost bit in current byte to be set
		int m = min(n, b+1);        // number of bits to be set in current byte
		int bitseq = (1 << m)-1;    // sequence of n set bits (bits 0..n-1 are set)
		bitseq <<= b-m+1;           // move bit sequence so that bit b is the leftmost set bit
		*byte |= uint8_t(bitseq);   // apply bit sequence to current byte
		byte++;
		n -= m;
		col += m;
	}
}


void Bitmap::forAllPixels (Callback &data) const {
	for (int row=0; row < _rows ; row++) {
		for (int col=0; col < _bpr; col++) {
			uint8_t byte = _bytes[row*_bpr+col];
			int x;
			for (int b=7; (b >= 0) && ((x = 8*col+(7-b)) < _cols); b--)
				data.pixel(x, row, bool(byte & (1 << b)), *this);
		}
	}
	data.finish();
}


class BBoxCallback : public Bitmap::Callback {
	public:
		int minx () const   {return _minx;}
		int miny () const   {return _miny;}
		int maxx () const   {return _maxx;}
		int maxy () const   {return _maxy;}
		bool empty () const {return !_changed;}

		void pixel (int x, int y, bool set, const Bitmap&) override {
			if (set) {
				_minx = min(_minx, x);
				_miny = min(_miny, y);
				_maxx = max(_maxx, x);
				_maxy = max(_maxy, y);
				_changed = true;
			}
		}

		void finish () override {
			if (empty())
				_minx = _miny = 0;
		}

	private:
		bool _changed = false;
		int _minx = numeric_limits<int>::max(), _miny=_minx;
		int _maxx = 0, _maxy = 0;
};


/** Computes the bounding box that spans all set pixels. */
bool Bitmap::getBBox (int &minx, int &miny, int &maxx, int &maxy) const {
	BBoxCallback bboxCallback;
	forAllPixels(bboxCallback);
	minx = bboxCallback.minx();
	miny = bboxCallback.miny();
	maxx = bboxCallback.maxx();
	maxy = bboxCallback.maxy();
	return !bboxCallback.empty();
}


/** Computes width and height of the bounding box that spans all set pixels. */
void Bitmap::getExtent (int &w, int &h) const {
	int minx, miny, maxx, maxy;
	if (getBBox(minx, miny, maxx, maxy)) {
		w = maxx-minx+1;
		h = maxy-miny+1;
	}
	else
		w = h = 0;
}


#if 0
ostream& Bitmap::write (ostream &os) const {
	for (int r=_rows-1; r >= 0 ; r--) {
		for (int c=0; c < _bpr; c++) {
			uint8_t byte = _bytes[r*_bpr+c];
			for (int b=128; b; b>>=1)
				os << (byte & b ? '*' : '-');
			os << ' ';
		}
		os << endl;
	}
	return os;
}
#endif
