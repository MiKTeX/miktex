/*************************************************************************
** PageSize.cpp                                                         **
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
#include <cctype>
#include <cmath>
#include <sstream>
#include "PageSize.hpp"
#include "utility.hpp"

using namespace std;


/** Computes width and height of ISO/DIN An.
 *  @param[in]  n the A level (e.g. n=4 => DIN A4)
 *  @param[out] width contains the page width when function returns
 *  @param[out] height contains the page height when function returns */
static void computeASize (int n, Length &width, Length &height) {
	height.set(round(1189.0/pow(math::SQRT2, n)), Length::Unit::MM);
	width.set(round(height.mm()/math::SQRT2), Length::Unit::MM);
}


/** Computes width and height of ISO/DIN Bn.
 *  @param[in]  n the B level (e.g. n=4 => DIN B4)
 *  @param[out] width contains the page width when function returns
 *  @param[out] height contains the page height when function returns */
static void computeBSize (int n, Length &width, Length &height) {
	Length w, h;
	computeASize(n, width, height);
	computeASize(n-1, w, h);
	width.set(round(sqrt(width.mm() * w.mm())), Length::Unit::MM);
	height.set(round(sqrt(height.mm() * h.mm())), Length::Unit::MM);
}


/** Computes width and height of ISO/DIN Cn.
 *  @param[in] n the C level (e.g. n=4 => DIN C4)
 *  @param[out] width contains the page width when function returns
 *  @param[out] height contains the page height when function returns */
static void computeCSize (int n, Length &width, Length &height) {
	Length w, h;
	computeASize(n, width, height);
	computeBSize(n, w, h);
	width.set(round(sqrt(width.mm() * w.mm())), Length::Unit::MM);
	height.set(round(sqrt(height.mm() * h.mm())), Length::Unit::MM);
}


/** Computes width and height of ISO/DIN Dn in millimeters.
 *  @param[in] n the D level (e.g. n=4 => DIN D4)
 *  @param[out] width contains the page width when function returns
 *  @param[out] height contains the page height when function returns */
static void computeDSize (int n, Length &width, Length &height) {
	Length w, h;
	computeASize(n, width, height);
	computeBSize(n+1, w, h);
	width.set(round(sqrt(width.mm() * w.mm())), Length::Unit::MM);
	height.set(round(sqrt(height.mm() * h.mm())), Length::Unit::MM);
}


/** Constructs a PageSize object of given size.
 *  @param[in] name specifies the page size, e.g. "A4" or "letter" */
PageSize::PageSize (const string &name) : _width(0), _height(0) {
	resize(name);
}


void PageSize::resize (Length w, Length h) {
	_width = w;
	_height = h;
}


void PageSize::resize (string name) {
	if (name.length() < 2)
		throw PageSizeException("unknown page format: "+name);

	name = util::tolower(name);
	// extract optional suffix
	size_t pos = name.rfind('-');
	bool landscape = false;
	if (pos != string::npos) {
		string suffix = name.substr(pos);
		name = name.substr(0, pos);
		if (suffix == "-l" || suffix == "-landscape")
			landscape = true;
		else if (suffix != "-p" && suffix != "-portrait")
			throw PageSizeException("invalid page format suffix: " + suffix);
	}

	if (name == "invoice") {
		_width = 140_mm;
		_height = 216_mm;
	}
	else if (name == "executive") {
		_width = 184_mm;
		_height = 267_mm;
	}
	else if (name == "legal") {
		_width = 216_mm;
		_height = 356_mm;
	}
	else if (name == "letter") {
		_width = 216_mm;
		_height = 279_mm;
	}
	else if (name == "ledger") {
		_width = 279_mm;
		_height = 432_mm;
	}
	else if (isdigit(name[1]) && name.length() < 5) {  // limit length of number to prevent arithmetic errors
		istringstream iss(name.substr(1));
		int n;
		iss >> n;
		switch (name[0]) {
			case 'a' : computeASize(n, _width, _height); break;
			case 'b' : computeBSize(n, _width, _height); break;
			case 'c' : computeCSize(n, _width, _height); break;
			case 'd' : computeDSize(n, _width, _height); break;
			default  : throw PageSizeException("invalid page format: "+name);
		}
	}
	if (_width.pt() == 0 || _height.pt() == 0)
		throw PageSizeException("unknown page format: "+name);
	if (landscape)
		swap(_width, _height);
}
