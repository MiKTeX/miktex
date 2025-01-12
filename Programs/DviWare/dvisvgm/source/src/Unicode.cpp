/*************************************************************************
** Unicode.cpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
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
 *  their usage, are mapped to the Private Use Zone U+E000-U+F8FF.
 *  @param[in] c character code to map
 *  @param[in] permitSpace if true, space characters are treated as allowed code points
 *  @return the code point */
uint32_t Unicode::charToCodepoint (uint32_t c, bool permitSpace) {
	static uint32_t ranges[] = {
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
	if (!permitSpace || c != 0x20) {
		for (size_t i=0; i < sizeof(ranges)/sizeof(uint32_t) && c >= ranges[i]; i+=3) {
			if (c <= ranges[i+1])
				return ranges[i+2]+c-ranges[i];
		}
	}
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


uint32_t Unicode::utf8ToCodepoint (const string &utf8) {
	auto len = utf8.length();
	if (len > 0) {
		unsigned char c0 = utf8[0];
		if (c0 <= 127)
			return c0;
		if (len > 1) {
			unsigned char c1 = utf8[1];
			if (c0 >= 0xC0 && c0 <= 0xDF)
				return ((c0-0xC0) << 6) + (c1-0x80);
			if (len > 2 && (c0 != 0xED || (c1 & 0xA0) != 0xA0)) {
				unsigned char c2 = utf8[2];
				if (c0 >= 0xE0 && c0 <= 0xEF)
					return ((c0-0xE0) << 12) + ((c1-0x80) << 6) + (c2-0x80);
				if (len > 3) {
					unsigned char c3 = utf8[3];
					if (c0 >= 0xF0 && c0 <= 0xF7)
						return  ((c0-0xF0) << 18) + ((c1-0x80) << 12) + ((c2-0x80) << 6) + (c3-0x80);
				}
			}
		}
	}
	return 0;
}


/** Converts a surrogate pair to its code point.
 *  @param[in] high high-surrogate value (upper 16 bits)
 *  @param[in] low low-surrogate value (lower 16 bits)
 *  @return corresponding code point or 0 if the surrogate is invalid */
uint32_t Unicode::fromSurrogate (uint32_t high, uint32_t low) {
	if (high < 0xD800 || high > 0xDBff || low < 0xDC00 || low > 0xDFFF)
		return 0;
	// http://www.unicode.org/versions/Unicode3.0.0/ch03.pdf, p. 45
	return (high-0xD800)*0x400 + low-0xDC00 + 0x10000;
}


/** Converts a surrogate value to its code point.
 *  @param[in] surrogate combined high and low surrogate value
 *  @return corresponding code point or 0 if the surrogate is invalid */
uint32_t Unicode::fromSurrogate (uint32_t surrogate) {
	return fromSurrogate(surrogate >> 16, surrogate & 0xFFFF);
}


/** Converts a code point of the surrogate range (0x10000--0x10FFFF)
 *  to its surrogate value.
 *  @param[in] cp code point to convert
 *  @return 32-bit surrogate (combined high and low values) */
uint32_t Unicode::toSurrogate (uint32_t cp) {
	if (cp < 0x10000 || cp > 0x10FFFF)
		return 0;
	// http://www.unicode.org/versions/Unicode3.0.0/ch03.pdf, p. 45
	uint32_t high = (cp-0x10000)/0x400 + 0xD800;
	uint32_t low = (cp-0x10000)%0x400 + 0xDC00;
	return (high << 16) | low;
}


uint32_t Unicode::toLigature (const string &nonlig) {
	struct Ligature {
		const char *nonlig;
		uint32_t lig;
	} ligatures[39] = {
		{u8"AA",  0xA732}, {u8"aa", 0xA733},
		{u8"AE",  0x00C6}, {u8"ae", 0x00E6},
		{u8"AO",  0xA734}, {u8"ao", 0xA735},
		{u8"AU",  0xA736}, {u8"au", 0xA737},
		{u8"AV",  0xA738}, {u8"av", 0xA739},
		{u8"AY",  0xA73C}, {u8"ay", 0xA73D},
		{u8"et", 0x1F670},
		{u8"ff",  0xFB00},
		{u8"ffi", 0xFB03},
		{u8"ffl", 0xFB04},
		{u8"fi",  0xFB01},
		{u8"fl",  0xFB02},
		{u8"Hv",  0x01F6}, {u8"hv", 0x0195},
		{u8"lb",  0x2114},
		{u8"lL",  0x1EFA}, {u8"ll", 0x1EFB},
		{u8"OE",  0x0152}, {u8"oe", 0x0153},
		{u8"OO",  0xA74E}, {u8"oo", 0xA74F},
		{u8"OO",  0xA74E},
		{u8"\u0254e", 0xAB62},
		{u8"\u017Fs", 0x1E9E}, {u8"\u017Az", 0x00DF},
		{u8"Tz",  0xA728}, {u8"tz",  0xA729},
		{u8"ue",  0x1D6B},
		{u8"uo",  0xAB63},
		{u8"VV",  0x0057}, {u8"tz",  0x0077},
		{u8"VY",  0xA760}, {u8"tz",  0xA761},
	};
	auto it = find_if(begin(ligatures), end(ligatures), [&nonlig](const Ligature &l) {
		return l.nonlig == nonlig;
	});
	return it != end(ligatures) ? it->lig : 0;
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
		hexstr.resize(4);
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
	auto pos = name.rfind('.');
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
