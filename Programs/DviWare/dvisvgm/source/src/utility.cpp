/*************************************************************************
** utility.cpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <functional>
#include "utility.hpp"

using namespace std;


/** Returns a given string with leading and trailing whitespace removed.
 *  @param[in] str the string to process
 *  @param[in] ws characters treated as whitespace
 *  @return the trimmed string */
string util::trim (const std::string &str, const char *ws) {
	size_t first = str.find_first_not_of(ws);
	if (first == string::npos)
		return "";
	size_t last = str.find_last_not_of(ws);
	return str.substr(first, last-first+1);
}


/** Removes leading and trailing whitespace from a given string, and replaces
 *  all other whitespace sequences by single spaces.
 *  @param[in] str the string to process
 *  @param[in] ws characters treated as whitespace
 *  @return the normalized string */
string util::normalize_space (string str, const char *ws) {
	str = trim(str);
	size_t first = str.find_first_of(ws);
	while (first != string::npos) {
		size_t last = str.find_first_not_of(ws, first);
		str.replace(first, last-first, " ");
		first = str.find_first_of(ws, first+1);
	}
	return str;
}


string& util::tolower (string &str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}


/** Returns the integer part of log10 of a given integer \f$n>0\f$.
 *  If \f$n<0\f$, the result is 0. */
int util::ilog10 (int n) {
	int result = 0;
	while (n >= 10) {
		result++;
		n /= 10;
	}
	return result;
}
