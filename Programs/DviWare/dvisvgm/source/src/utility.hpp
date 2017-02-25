/*************************************************************************
** utility.hpp                                                          **
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

#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string>

namespace math {

constexpr const double PI      = 3.141592653589793238462643383279502884;
constexpr const double HALF_PI = 1.570796326794896619231321691639751442;
constexpr const double TWO_PI  = 6.283185307179586476925286766559005768;


inline double deg2rad (double deg) {return PI*deg/180.0;}

} // namespace math

namespace util {

std::string trim (const std::string &str, const char *ws=" \t\n\r\f");
std::string normalize_space (std::string str, const char *ws=" \t\n\r\f");
std::string& tolower (std::string &str);
int ilog10 (int n);


/** Encodes the bytes in the half-open range [first,last) to Base64 and writes
 *  the result to the range starting at 'dest'.
 *  @param[in] first initial position of the range to be encoded
 *  @param[in] last final position of the range to be encoded
 *  @param[in] dest first position of the destination range */
template <typename InputIterator, typename OutputIterator>
void base64_copy (InputIterator first, InputIterator last, OutputIterator dest) {
	static const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	while (first != last) {
		int padding = 0;
		unsigned char c0 = *first++, c1=0, c2=0;
		if (first == last)
			padding = 2;
		else {
			c1 = *first++;
			if (first == last)
				padding = 1;
			else
				c2 = *first++;
		}
		uint32_t n = (c0 << 16) | (c1 << 8) | c2;
		for (int i=0; i <= 3-padding; i++) {
			*dest++ = base64_chars[(n >> 18) & 0x3f];
			n <<= 6;
		}
		while (padding--)
			*dest++ = '=';
	}
}

} // namespace util

#endif
