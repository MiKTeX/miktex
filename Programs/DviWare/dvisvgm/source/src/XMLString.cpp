/*************************************************************************
** XMLString.cpp                                                        **
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
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include "Unicode.hpp"
#include "XMLString.hpp"

using namespace std;

int XMLString::DECIMAL_PLACES = 0;


static string translate (uint32_t c) {
	switch (c) {
		case '<' : return "&lt;";
		case '&' : return "&amp;";
		case '"' : return "&quot;";
		case '\'': return "&apos;";
	}
	return Unicode::utf8(c);
}


XMLString::XMLString (const string &str, bool plain) {
	if (plain)
		assign(str);
	else {
		for (char c : str)
			*this += translate(c);
	}
}


XMLString::XMLString (const char *str, bool plain) {
	if (str) {
		if (plain)
			assign(str);
		else {
			while (*str)
				*this += translate(*str++);
		}
	}
}


XMLString::XMLString (int n, bool cast) {
	if (cast) {
		stringstream ss;
		ss << n;
		ss >> *this;
	}
	else
		*this += translate(n);
}


/** Rounds a floating point value to a given number of decimal places.
 *  @param[in] x number to round
 *  @param[in] n number of decimal places (must be between 1 and 6)
 *  @return rounded value */
static inline double round (double x, int n) {
	const long pow10[] = {10L, 100L, 1000L, 10000L, 100000L, 1000000L};
	const double eps = 1e-7;
	n--;
	if (x >= 0)
		return floor(x*pow10[n]+0.5+eps)/pow10[n];
	return ceil(x*pow10[n]-0.5-eps)/pow10[n];
}


XMLString::XMLString (double x) {
	stringstream ss;
	if (fabs(x) < 1e-8)
		x = 0;
	if (DECIMAL_PLACES > 0)
		x = round(x, DECIMAL_PLACES);
	// don't use fixed and setprecision() manipulators here to avoid
	// banker's rounding applied in some STL implementations
	ss << x;
	ss >> *this;
}
