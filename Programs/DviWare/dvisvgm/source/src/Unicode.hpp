/*************************************************************************
** Unicode.hpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef UNICODE_HPP
#define UNICODE_HPP

#include <cstdint>
#include <string>

struct Unicode {
	static bool isValidCodepoint (uint32_t code);
	static uint32_t charToCodepoint (uint32_t c, bool permitSpace=false);
	static std::string utf8 (int32_t c);
	static uint32_t utf8ToCodepoint (const std::string &utf8);
	static uint32_t fromSurrogate (uint32_t high, uint32_t low);
	static uint32_t fromSurrogate (uint32_t cp);
	static uint32_t toSurrogate (uint32_t cp);
	static uint32_t toLigature (const std::string &nonlig);
	static int32_t aglNameToCodepoint (const std::string &name);
};

#endif
