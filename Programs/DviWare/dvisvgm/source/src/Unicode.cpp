/*************************************************************************
** Unicode.cpp                                                          **
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
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <xxhash.h>
#include "Unicode.hpp"

using namespace std;


/** Returns true if c is a valid Unicode point in XML documents.
 *  XML version 1.0 doesn't allow various Unicode character references
 *  (&#1; for example). */
bool Unicode::isValidCodepoint (uint32_t c) {
	if ((c & 0xffff) == 0xfffe || (c & 0xffff) == 0xffff)
		return false;

	uint32_t ranges[] = {
		0x0000, 0x0020,  // basic control characters + space
		0x007f, 0x009f,  // use of control characters is discouraged by the XML standard
		0x202a, 0x202e,  // bidi control characters
		0xd800, 0xdfff,  // High Surrogates are not allowed in XML
		0xfdd0, 0xfdef,  // non-characters for internal use by applications
	};
	for (size_t i=0; i < sizeof(ranges)/sizeof(uint32_t) && c >= ranges[i]; i+=2)
		if (c <= ranges[i+1])
			return false;
	return true;
}


/** Returns a valid Unicode point for the given character code. Character codes
 *  that are invalid code points because the XML standard forbids or discourages
 *  their usage, are mapped to the Private Use Zone U+E000-U+F8FF. */
uint32_t Unicode::charToCodepoint (uint32_t c) {
	uint32_t ranges[] = {
		0x0000, 0x0020, 0xe000, // basic control characters + space
		0x007f, 0x009f, 0xe021, // use of control characters is discouraged by the XML standard
		0x202a, 0x202e, 0xe042, // bidi control characters
		0xd800, 0xdfff, 0xe047, // High Surrogates are not allowed in XML
		0xfdd0, 0xfdef, 0xe847, // non-characters for internal use by applications
		0xfffe, 0xffff, 0xe867,
		0x1fffe, 0x1ffff, 0xe869,
		0x2fffe, 0x2ffff, 0xe86b,
		0x3fffe, 0x3ffff, 0xe86d,
		0x4fffe, 0x4ffff, 0xe86f,
		0x5fffe, 0x5ffff, 0xe871,
		0x6fffe, 0x6ffff, 0xe873,
		0x7fffe, 0x7ffff, 0xe875,
		0x8fffe, 0x8ffff, 0xe877,
		0x9fffe, 0x9ffff, 0xe879,
		0xafffe, 0xaffff, 0xe87b,
		0xbfffe, 0xbffff, 0xe87d,
		0xcfffe, 0xcffff, 0xe87f,
		0xdfffe, 0xdffff, 0xe881,
		0xefffe, 0xeffff, 0xe883,
		0xffffe, 0xfffff, 0xe885,
		0x10fffe, 0x10ffff, 0xe887
	};
	for (size_t i=0; i < sizeof(ranges)/sizeof(unsigned) && c >= ranges[i]; i+=3)
		if (c <= ranges[i+1])
			return ranges[i+2]+c-ranges[i];
	return c;
}


/** Converts a Unicode point to a UTF-8 byte sequence.
 *  @param[in] cp code point
 *  @return  utf8 sequence consisting of 1-4 bytes */
string Unicode::utf8 (int32_t cp) {
	string utf8;
	if (cp >= 0) {
		if (cp < 0x80)
			utf8 += char(cp);
		else if (cp < 0x800) {
			utf8 += char(0xC0 + (cp >> 6));
			utf8 += char(0x80 + (cp & 0x3F));
		}
		else if (cp < 0x10000) {
			utf8 += char(0xE0 + (cp >> 12));
			utf8 += char(0x80 + ((cp >> 6) & 0x3F));
			utf8 += char(0x80 + (cp & 0x3F));
		}
		else if (cp < 0x110000) {
			utf8 += char(0xF0 + (cp >> 18));
			utf8 += char(0x80 + ((cp >> 12) & 0x3F));
			utf8 += char(0x80 + ((cp >> 6) & 0x3F));
			utf8 += char(0x80 + (cp & 0x3F));
		}
		// UTF-8 does not support codepoints >= 0x110000
	}
	return utf8;
}

#include "AGLTable.hpp"

/** Tries to extract the codepoint from AGL character names like "uni1234" or "u1234".
 *  Returns 0 if the given name doesn't satisfy the constraints.
 *  https://github.com/adobe-type-tools/agl-specification
 *  @param[in] name AGL character name
 *  @return the extracted codepoint or 0 on failure */
static int32_t extract_codepoint_from_name (const string &name) {
	size_t offset=1;
	auto is_hex_digit = [](char c) {return isdigit(c) || (c >= 'A' && c <= 'F');};
	if (name.substr(0, 3) == "uni" && is_hex_digit(name[4]) && name.length() >= 7)
		offset = 3;
	else if (name[0] != 'u' || !is_hex_digit(name[1]) || name.length() < 5)
		return 0;

	string::const_iterator it = name.begin()+offset;
	while (it != name.end() && is_hex_digit(*it) && *it != '.' && *it != '_')
		++it;
	if (it != name.end() && *it != '.' && *it != '_')
		return 0;

	string hexstr(name.begin()+offset, it);
	if (hexstr.length() < 4 || (offset == 3 && hexstr.length() % 4 != 0))
		return 0;
	if (offset == 3)
		hexstr = hexstr.substr(0, 4);
	int32_t codepoint;
	istringstream iss(hexstr);
	iss >> hex >> codepoint;
	if (!iss.fail() && (codepoint <= 0xD7FF || (codepoint >= 0xE000 && codepoint <= 0x10FFFF)))
		return codepoint;
	return 0;
}


#if 0
static const char* get_suffix (const string &name) {
	static const char *suffixes[] = {
		"small", "swash", "superior", "inferior", "numerator", "denominator", "oldstyle",
		"display", "text", "big", "bigg", "Big", "Bigg", 0
	};
	size_t pos = name.rfind('.');
	if (pos != string::npos) {
		string suffix = name.substr(pos+1);
		for (const char **p=suffixes; *p; p++)
			if (suffix == *p)
				return *p;
	}
	return 0;
}
#endif


/** Returns the Unicode point for a given AGL character name.
 * @param name AGL name of the character to look up
 * @return codepoint of the character */
int32_t Unicode::aglNameToCodepoint (const string &name) {
	if (int32_t cp = extract_codepoint_from_name(name))
		return cp;

	uint32_t hash = XXH32(&name[0], name.length(), 0);
	const HashCodepointPair cmppair = {hash, 0};
	auto it = lower_bound(hash2unicode.begin(), hash2unicode.end(), cmppair,
		[](const HashCodepointPair &p1, const HashCodepointPair &p2) {
			return p1.hash < p2.hash;
		}
	);
	if (it != hash2unicode.end() && it->hash == hash)
		return it->codepoint;
	return 0;
}
