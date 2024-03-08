/*************************************************************************
** OS2Table.cpp                                                         **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include "OS2Table.hpp"
#include "TTFWriter.hpp"
#include "../Font.hpp"

using namespace std;
using namespace ttf;


/** Returns the average width of all non-zero width glyphs.
 *  https://docs.microsoft.com/en-us/typography/opentype/spec/os2#acw */
int16_t OS2Table::averageCharWidth () const {
	const RangeMap &charmap = ttfWriter()->getUnicodeCharMap();
	if (charmap.empty())
		return 0;
	int sum=ttfWriter()->hAdvance(0);  // width of .notdef character
	int count=1;
	for (auto uc2charcode : charmap) {
		int c = int(uc2charcode.second);
		if (int w = ttfWriter()->hAdvance(c)) {
			sum += w;
			count++;
		}
	}
	return int16_t(double(sum)/count);
}

static vector<uint32_t> compute_unicode_range_bits (const RangeMap &charmap);


void OS2Table::write (ostream &os) const {
	const PhysicalFont &font = ttfWriter()->getFont();
	int upem = ttfWriter()->targetUnitsPerEm();
	double scale = ttfWriter()->unitsPerEmFactor();
	const RangeMap &charmap = ttfWriter()->getUnicodeCharMap();

	writeUInt16(os, 3);   // table version
	writeInt16(os, averageCharWidth());  // average char width
	writeUInt16(os, 400);   // usWeightClass, "normal" for now
	writeUInt16(os, 5);     // usWidthClass, "medium" for now
	writeUInt16(os, 0);     // fsType
	writeInt16(os, round(0.65*upem));  // ySubscriptXSize
	writeInt16(os, round(0.7*upem));   // ySubscriptYSize
	writeInt16(os, round(-0.48*upem)); // ySubscriptXOffset
	writeInt16(os, round(0.14*upem));  // ySubscriptYOffset
	writeInt16(os, round(0.65*upem));  // ySuperscriptXSize
	writeInt16(os, round(0.7*upem));   // ySuperscriptYSize
	writeInt16(os, round(0.48*upem));  // ySuperscriptXOffset
	writeInt16(os, round(0.48*upem));  // ySuperscriptYOffset
	writeInt16(os, 102*upem/2048);     // yStrikeoutSize
	writeInt16(os, 530*upem/2048);     // yStrikeoutPosition
	writeInt16(os, 0);                 // sFamilyClass (no classification)
	for (uint8_t p : {0, 0, 6, 0, 0, 0, 0, 0, 0, 0})  // Panose
		writeUInt8(os, p);
	for (uint32_t bits : compute_unicode_range_bits(charmap))
		writeUInt32(os, bits);
	writeUInt32(os, name2id("dsvg"));
	writeUInt16(os, 6);        // fsSelection, "regular" for now
	writeUInt16(os, min(charmap.minKey(), uint32_t(0xFFFF)));  // usFirstCharIndex
	writeUInt16(os, min(charmap.maxKey(), uint32_t(0xFFFF)));  // usLastCharIndex
	writeInt16(os, round(scale*font.ascent()));   // sTypoAscender
	writeInt16(os, round(-scale*font.descent())); // sTypoDescender
	writeInt16(os, 0);        // sTypoLineGap
	writeUInt16(os, round(scale*font.ascent()));
	writeUInt16(os, round(scale*font.descent()));
	writeUInt32(os, 1);  // ulCodePageRange1, "Latin 1" for now
	writeUInt32(os, 0);  // ulCodePageRange2
	writeInt16(os, 0);   // sxHeight
	writeInt16(os, charmap.valueExists(0x48) ? 0x48 : 0);   // sCapHeight
	writeUInt16(os, 0);  // usDefaultChar
	writeUInt16(os, charmap.valueExists(0x20) ? 0x20 : 0xffff);  // usBreakChar (space or .notdef)
	writeUInt16(os, 0);  // usMaxContext
}

#if 0
void OS2Table::updateYMinMax (int16_t ymin, int16_t ymax) {
	_usWinAscent = max(_usWinAscent, uint16_t(ymax));
	_usWinDescent = max(_usWinDescent, uint16_t(ymin < 0 ? -ymin : 0));
}
#endif


/** Returns the bit number denoting the Unicode range for a codepoint according to
 *  the OS/2 table specification. If there's no matching range for the codepoint,
 *  the function returns -1. Currently, only the ranges specified by OS/2 table
 *  version 1 are considered. */
static int unicode_range_bit (uint32_t codepoint) {
	struct UCRange {
		UCRange (uint32_t cp) : first(cp), last(cp), os2bit(0) {}
		UCRange (uint32_t cp1, uint32_t cp2, int bit) : first(cp1), last(cp2), os2bit(bit) {}
		uint32_t first, last;
		int os2bit;
	};
	// Unicode ranges according to http://unicode.org/Public/UNIDATA/Blocks.txt
	static array<UCRange, 100> ucranges {{
		{0x00020, 0x0007E,   0},  // Basic Latin
		{0x000A0, 0x000FF,   1},  // Latin-1 Supplement
		{0x00100, 0x0017F,   2},  // Latin Extended-A
		{0x00180, 0x0024F,   3},  // Latin Extended-B
		{0x00250, 0x002AF,   4},  // IPA Extensions
		{0x002B0, 0x002FF,   5},  // Spacing Modifier Letters
		{0x00300, 0x0036F,   6},  // Combining Diacritical Marks
		{0x00370, 0x003FF,   7},  // Greek and Coptic
		{0x00400, 0x0052F,   9},  // Cyrillic / Cyrillic Supplement
		{0x00530, 0x0058F,  10},  // Armenian
		{0x00590, 0x005FF,  11},  // Hebrew
		{0x00600, 0x006FF,  13},  // Arabic
		{0x00700, 0x0074F,  71},  // Syriac
		{0x00750, 0x0077F,  13},  // Arabic Supplement
		{0x00780, 0x007BF,  72},  // Thaana
		{0x007C0, 0x007FF,  14},  // N'Ko
		{0x00800, 0x0083F,  12},  // Samaritan
		{0x00900, 0x0097F,  15},  // Devanagari
		{0x00980, 0x009FF,  16},  // Bengali
		{0x00A00, 0x00A7F,  17},  // Gurmukhi
		{0x00A80, 0x00AFF,  18},  // Gujarati
		{0x00B00, 0x00B7F,  19},  // Oriya
		{0x00B80, 0x00BFF,  20},  // Tamil
		{0x00C00, 0x00C7F,  21},  // Telugu
		{0x00C80, 0x00CFF,  22},  // Kannada
		{0x00D00, 0x00D7F,  23},  // Malayalam
		{0x00E00, 0x00E7F,  24},  // Thai
		{0x00E80, 0x00EFF,  25},  // Lao
		{0x010A0, 0x010FF,  26},  // Georgian
		{0x01100, 0x011FF,  28},  // Hangul Jamo
		{0x01B00, 0x01B7F,  27},  // Balinese
		{0x01D00, 0x01DBF,   4},  // Phonetic Extensions & Supplement
		{0x01D80, 0x01DFF,   6},  // Combining Diacritical Marks Supplement
		{0x01E00, 0x01EFF,  29},  // Latin Extended Additional
		{0x01F00, 0x01FFF,  30},  // Greek Extended
		{0x02000, 0x0206F,  31},  // General Punctuation
		{0x02070, 0x0209F,  32},  // Superscripts and Subscripts
		{0x020A0, 0x020CF,  33},  // Currency Symbols
		{0x020D0, 0x020FF,  34},  // Combining Marks for Symbols
		{0x02100, 0x0214F,  35},  // Letterlike Symbols
		{0x02150, 0x0218F,  36},  // Number Forms
		{0x02190, 0x021FF,  37},  // Arrows
		{0x02200, 0x022FF,  38},  // Mathematical Operators
		{0x02300, 0x0237F,  39},  // Miscellaneous Technical
		{0x02400, 0x0243F,  40},  // Control Pictures
		{0x02440, 0x0245F,  41},  // Optical Character Recognition
		{0x02460, 0x024FF,  42},  // Enclosed Alphanumerics
		{0x02500, 0x0257F,  43},  // Box Drawing
		{0x02580, 0x0259F,  44},  // Block Elements
		{0x025A0, 0x025FF,  45},  // Geometric Shapes
		{0x02600, 0x0267F,  46},  // Miscellaneous Symbols
		{0x02700, 0x027BF,  47},  // Dingbats
		{0x027C0, 0x027EF,  38},  // Miscellaneous Mathematical Symbols-A
		{0x027F0, 0x027FF,  37},  // Supplementary Arrows-A
		{0x02800, 0x028FF,  82},  // Braille Patterns
		{0x02900, 0x0297F,  37},  // Supplementary Arrows-B
		{0x02980, 0x02AFF,  38},  // Miscellaneous Mathematical Symbols-B + Supplemental Mathematical Operators
		{0x02B00, 0x02BFF,  37},  // Miscellaneous Symbols and Arrows
		{0x02C60, 0x02C7F,  29},  // Latin Extended-C
		{0x02C80, 0x02CFF,   8},  // Coptic
		{0x02D00, 0x02D25,  26},  // Georgian Supplement
		{0x02DE0, 0x02DFF,   9},  // Cyrillic Extended-A
		{0x02E00, 0x02E7F,  31},  // Supplemental Punctuation
		{0x02E80, 0x02FFF,  59},  // CJK Radicals Supplement + Kangxi Radicals + Ideographic Description Characters
		{0x03000, 0x0303F,  48},  // CJK Symbols and Punctuation
		{0x03040, 0x0309F,  49},  // Hiragana
		{0x030A0, 0x030FF,  50},  // Katakana
		{0x03100, 0x0312F,  51},  // Bopomofo
		{0x03130, 0x0318F,  52},  // Hangul Compatibility Jamo
		{0x03190, 0x0319F,  59},  // Kanbun
		{0x031A0, 0x031BF,  51},  // Bopomofo Extended
		{0x031F0, 0x031FF,  50},  // Katakana Phonetic Extensions
		{0x03200, 0x032FF,  54},  // Enclosed CJK Letters and Months
		{0x03300, 0x033FF,  55},  // CJK compatability
		{0x03400, 0x04DBF,  59},  // CJK Unified Ideographs Extension A
		{0x04E00, 0x09FFF,  59},  // CJK Unified Ideographs
		{0x0A500, 0x0A62B,  12},  // Vai
		{0x0A640, 0x0A69F,   9},  // Cyrillic Extended-B
		{0x0A700, 0x0A71F,   5},  // Modifier Tone Letters
		{0x0A720, 0x0A7FF,  29},  // Latin Extended-D
		{0x0A840, 0x0A87F,  53},  // Phags-pa
		{0x0AC00, 0x0D7AF,  56},  // Hangul Syllables
		{0x0D800, 0x0DFFF,  57},  // Non-Plane 0
		{0x0E000, 0x0F8FF,  60},  // Private Use Area
		{0x0F900, 0x0FAFF,  61},  // CJK Compatibility Ideographs
		{0x0FA2A, 0x0FAFF,  61},  // CJK Compatibility Ideographs
		{0x0FB00, 0x0FB4F,  62},  // Alphabetic Presentation Forms
		{0x0FB50, 0x0FDFF,  63},  // Arabic Presentation Forms-A
		{0x0FE00, 0x0FE0F,  91},  // Variation Selectors
		{0x0FE20, 0x0FE2F,  64},  // Combining Half Marks
		{0x0FE30, 0x0FE4F,  65},  // CJK Compatibility Forms
		{0x0FE50, 0x0FE6F,  66},  // Small Form Variants
		{0x0FE70, 0x0FEEF,  67},  // Arabic Presentation Forms-B
		{0x0FF00, 0x0FFEF,  68},  // Halfwidth and Fullwidth Forms
		{0x0FFF0, 0x0FFFF,  69},  // Specials
		{0x10900, 0x1091F,  58},  // Phoenician
		{0x20000, 0x2A6DF,  59},  // CJK Unified Ideographs Extension B
		{0x2A700, 0x2B73F,  59},  // CJK Unified Ideographs Extension C
		{0x2B740, 0x2B81F,  59},  // CJK Unified Ideographs Extension D
		{0x2F800, 0x2FA1F,  61},  // CJK Compatibility Ideographs Supplement
	}};
	auto it = lower_bound(ucranges.begin(), ucranges.end(), UCRange(codepoint), [](const UCRange &r1, const UCRange &r2) {
		return r1.last < r2.first;
	});
	if (it != ucranges.end() && codepoint >= it->first && codepoint <= it->last)
		return it->os2bit;
	return -1;
}


/** Returns the Unicode ranges encompassed by the font. The ranges are encoded by four dwords,
 *  where each of the 128 bits represents a certain range. The returned vector contains the
 *  dwords in little-endian order, i.e. [bits 0-31, bits 32-63, bits 64-95, bits 96-127]. */
static vector<uint32_t> compute_unicode_range_bits (const RangeMap &charmap) {
	vector<uint32_t> rangeBits(4, 0);
	for (auto uc2charcode : charmap) {
		int bit = unicode_range_bit(uc2charcode.first);
		if (bit >= 0) {
			for (int i=0; i < 4; i++) {
				if (bit < 32*(i+1)) {
					rangeBits[i] |= (1 << (bit-32*i));
					break;
				}
			}
		}
	}
	return rangeBits;
}
