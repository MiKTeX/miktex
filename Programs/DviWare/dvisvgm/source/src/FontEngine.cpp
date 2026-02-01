/*************************************************************************
** FontEngine.cpp                                                       **
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
#include <cmath>
#include <set>
#include <sstream>
#include <ft2build.h>
#include FT_ADVANCES_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TYPES_H
#include "Font.hpp"
#include "FontEngine.hpp"
#include "FontStyle.hpp"
#include "fonts/Base14Fonts.hpp"
#include "Message.hpp"
#include "Unicode.hpp"
#include "utility.hpp"

using namespace std;


/** Converts a floating point value to a 16.16 fixed point value. */
static inline FT_Fixed to_16dot16 (double val) {
	return static_cast<FT_Fixed>(lround(val*65536.0));
}


/** Converts an integer to a 16.16 fixed point value. */
static inline FT_Fixed to_16dot16 (int val) {
	return static_cast<FT_Fixed>(val) << 16;
}


///////////////////////////////////////////////////////////////////////////


FontEngine::FontEngine () {
	if (FT_Init_FreeType(&_library))
		Message::estream(true) << "failed to initialize FreeType library\n";
}


FontEngine::~FontEngine () {
	if (_currentFace && FT_Done_Face(_currentFace))
		Message::estream(true) << "failed to release font\n";
	if (FT_Done_FreeType(_library))
		Message::estream(true) << "failed to release FreeType library\n";
}


/** Returns the singleton instance of this class. */
FontEngine& FontEngine::instance () {
	static FontEngine engine;
	return engine;
}


string FontEngine::version () {
	FT_Int major, minor, patch;
	FT_Library &lib = instance()._library;
	FT_Library_Version(lib, &major, &minor, &patch);
	ostringstream oss;
	oss << major << '.' << minor << '.' << patch;
	return oss.str();
}


/** Sets the font to be used.
 * @param[in] fname path to font file
 * @param[in] fontindex index of font in font collection (multi-font files, like TTC)
 * @param[in] charMapID character map ID to assign
 * @return true on success */
bool FontEngine::setFont (const string &fname, int fontindex, const CharMapID &charMapID) {
	if (_currentFace && FT_Done_Face(_currentFace))
		Message::estream(true) << "failed to release font\n";
	if (fname.size() <= 6 || fname.substr(0, 6) == "sys://") {
		if (const MemoryFontData *data = find_base14_font(fname.substr(6))) {
			FT_Open_Args args;
			args.flags = FT_OPEN_MEMORY;
			args.memory_base = reinterpret_cast<const FT_Byte*>(data->data);
			args.memory_size = FT_Long(data->size);
			if (FT_Open_Face(_library, &args, fontindex, &_currentFace)) {
				Message::estream(true) << "can't read memory font " << fname << '\n';
				return false;
			}
		}
	}
	else if (FT_New_Face(_library, fname.c_str(), fontindex, &_currentFace)) {
		Message::estream(true) << "can't read font file " << fname << '\n';
		return false;
	}
	if (charMapID.valid())
		setCharMap(charMapID);
	return true;
}


bool FontEngine::setFont (const Font &font) {
	if (_currentFont && _currentFont->name() == font.name())
		return true;

	if (const char *path=font.path()) {
		auto pf = font_cast<const PhysicalFont*>(&font);
		if (setFont(path, font.fontIndex(), pf ? pf->getCharMapID() : CharMapID())) {
			_currentFont = &font;
			return true;
		}
	}
	return false;
}


bool FontEngine::isCIDFont() const {
	FT_Bool cid_keyed;
	return _currentFace && FT_Get_CID_Is_Internally_CID_Keyed(_currentFace, &cid_keyed) == 0 && cid_keyed;
}


/** Returns true if the current font contains vertical layout data. */
bool FontEngine::hasVerticalMetrics () const {
	return _currentFace && FT_HAS_VERTICAL(_currentFace);
}


bool FontEngine::setCharMap (const CharMapID &charMapID) const {
	if (_currentFace) {
		for (int i = 0; i < _currentFace->num_charmaps; i++) {
			FT_CharMap ft_cmap = _currentFace->charmaps[i];
			if (ft_cmap->platform_id == charMapID.platform_id && ft_cmap->encoding_id == charMapID.encoding_id) {
				FT_Set_Charmap(_currentFace, ft_cmap);
				return true;
			}
		}
	}
	return false;
}


/** Returns a character map that maps from glyph indexes to Unicode code points */
RangeMap FontEngine::buildGidToUnicodeMap () const {
	RangeMap charmap;
	if (!_currentFace)
		return charmap;
	FT_CharMap ft_cmap = _currentFace->charmap;
	if (FT_Select_Charmap(_currentFace, FT_ENCODING_UNICODE) == 0) {
		FT_UInt gid;  // index of current glyph
		uint32_t charcode = FT_Get_First_Char(_currentFace, &gid);
		while (gid) {
			if (!charmap.valueExists(gid))
				charmap.addRange(gid, gid, charcode);
			charcode = FT_Get_Next_Char(_currentFace, charcode, &gid);
		}
		FT_Set_Charmap(_currentFace, ft_cmap);
	}
	// In case the Unicode map of the font doesn't cover all characters, some
	// of them could still be identified by their names if present in the font.
	if (FT_HAS_GLYPH_NAMES(_currentFace)) {
		NumericRanges<uint32_t> usedCodepoints;
		for (size_t i=0; i < charmap.numRanges(); i++)
			usedCodepoints.addRange(charmap.getRange(i).minval(), charmap.getRange(i).maxval());
		if (charmap.empty())
			addCharsByGlyphNames(1, getNumGlyphs(), charmap, usedCodepoints);
		else {
			addCharsByGlyphNames(1, charmap.minKey()-1, charmap, usedCodepoints);
			for (size_t i=1; i < charmap.numRanges(); i++)
				addCharsByGlyphNames(charmap.getRange(i-1).max()+1, charmap.getRange(i).min()-1, charmap, usedCodepoints);
			addCharsByGlyphNames(charmap.maxKey()+1, getNumGlyphs(), charmap, usedCodepoints);
		}
	}
	return charmap;
}


/** Looks for known glyph names in a given GID range and adds the corresponding
 *  GID->code point mapping to a character map if the code point is not already present.
 *  @param[in] minGID minimum GID of range to iterate
 *  @param[in] maxGID maximum GID of range to iterate
 *  @param[in,out] charmap character map taking the new mappings
 *  @param[in,out] ucp collection of code points already present in the character map */
void FontEngine::addCharsByGlyphNames (uint32_t minGID, uint32_t maxGID, RangeMap &charmap, CodepointRanges &ucp) const {
	for (uint32_t gid=minGID; gid <= maxGID; gid++) {
		char glyphName[64];
		if (FT_Get_Glyph_Name(_currentFace, gid, glyphName, 64) == 0) {
			const int32_t cp = Unicode::aglNameToCodepoint(glyphName);
			if (cp && !ucp.valueExists(cp)) {
				charmap.addRange(gid, gid, cp);
				ucp.addRange(cp);
			}
		}
	}
}


/** Returns a map that maps the character codes of the currently selected char map to their glyph IDs.
 *  @param[in] face font face to build the map for
 *  @param[out] gids all GIDs mapped to a char code
 *  @return char code to GID map based on the data present in the font */
static RangeMap build_charcode_to_gid_map (FT_Face face, set<uint32_t> &gids) {
	RangeMap charmap;
	FT_UInt gid;  // index of current glyph
	uint32_t charcode = FT_Get_First_Char(face, &gid);
	while (gid) {
		charmap.addRange(charcode, charcode, gid);
		gids.insert(gid);
		charcode = FT_Get_Next_Char(face, charcode, &gid);
	}
	return charmap;
}


/** Returns a map that maps specified glyph IDs to Unicode code points based on the data in the font.
 *  GIDs covered by the font's Unicode map are removed from set 'codePoints', the unmapped GIDs stay
 *  in the set.
 *  @param[in] face font face to build the map for
 *  @param[in,out] gids glyph IDs to process; GIDs present in the resulting map are removed from this set
 *  @param[out] codePoints code points successfully assigned to a GID
 *  @return GID to code point map based on the data present in the font */
static RangeMap build_gid_to_unicode_map (FT_Face face, set<uint32_t> &gids, set<uint32_t> &codePoints) {
	RangeMap gidToUnicodeMap;
	if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) == 0) {
		FT_UInt gid;  // index of current glyph
		uint32_t cp = FT_Get_First_Char(face, &gid);  // Unicode code point
		while (gid) {
			if (gids.find(gid) != gids.end()) {
				gidToUnicodeMap.addRange(gid, gid, cp);
				gids.erase(gid);
				codePoints.insert(cp);
			}
			cp = FT_Get_Next_Char(face, cp, &gid);
		}
		if (FT_HAS_GLYPH_NAMES(face)) {
			for (auto gidIt = gids.begin(); gidIt != gids.end();) {
				gid = *gidIt;
				cp = gidToUnicodeMap.valueAt(gid);
				if (!cp) {
					char glyphName[64];
					if (FT_Get_Glyph_Name(face, gid, glyphName, 64) == 0) {
						cp = Unicode::aglNameToCodepoint(glyphName);
						if (cp && codePoints.find(cp) == codePoints.end())
							gidToUnicodeMap.addRange(gid, gid, cp);
						else
							cp = 0;
					}
				}
				if (cp) {
					codePoints.insert(cp);
					gidIt = gids.erase(gidIt);
				}
				else ++gidIt;
			}
		}
	}
	return gidToUnicodeMap;
}


/** Creates a character map that maps from the custom character encoding to Unicode.
 *  @return pointer to character map if it could be created, 0 otherwise */
unique_ptr<const RangeMap> FontEngine::createCustomToUnicodeMap () const {
	if (!_currentFace)
		return nullptr;
	auto charmap = util::make_unique<RangeMap>();
	FT_CharMap ftcharmap = _currentFace->charmap;
	if (FT_Select_Charmap(_currentFace, FT_ENCODING_ADOBE_CUSTOM) != 0)
		return nullptr;

	set<uint32_t> assignedGIDs;  // all GIDs assigned to a char code
	const RangeMap charCodeToGIDMap = build_charcode_to_gid_map(_currentFace, assignedGIDs);

	// get the code points for all GIDs assigned to custom character codes
	set<uint32_t> assignedCodePoints;
	RangeMap gidToUnicodeMap = build_gid_to_unicode_map(_currentFace, assignedGIDs, assignedCodePoints);

	FT_Set_Charmap(_currentFace, ftcharmap);  // reassign initially active character map

	// assign remaining GIDs to code points from a Private Use Area (PUA)
	const NumericRanges<uint32_t> pua({{0xE000, 0xF8FF}, {0xF0000, 0xFFFFD}, {0x100000, 0x10FFFD}});
	auto puaIt = pua.valueIterator();
	for (const uint32_t gid : assignedGIDs) {
		for (; puaIt.valid(); ++puaIt) {
			if (assignedCodePoints.find(*puaIt) == assignedCodePoints.end()) {
				gidToUnicodeMap.addRange(gid, gid, *puaIt);
				assignedCodePoints.insert(*puaIt++);
				break;
			}
		}
	}
	assignedCodePoints.clear();
	// build the resulting map via char code -> GID -> code point
	for (const auto entry : charCodeToGIDMap) {
		const uint32_t charCode = entry.first;
		const uint32_t gid = entry.second;
		const uint32_t cp = gidToUnicodeMap.valueAt(gid);
		charmap->addRange(charCode, charCode, cp);
	}
	return charmap->empty() ? nullptr : std::move(charmap);
}


const char* FontEngine::getFamilyName () const {
	return _currentFace ? _currentFace->family_name : nullptr;
}


const char* FontEngine::getStyleName () const {
	return _currentFace ? _currentFace->style_name : nullptr;
}


/** Returns the PS name of the current font. */
const char* FontEngine::getPSName () const {
	return _currentFace ? FT_Get_Postscript_Name(_currentFace) : nullptr;
}


/** Returns the PS name of a font given by a file.
 *  @param[in] fname name/path of the font file
 *  @return the PS name */
string FontEngine::getPSName (const string &fname) const {
	string psname;
	FT_Face face;
	if (FT_New_Face(_library, fname.c_str(), 0, &face) == 0) {
		if (const char *ptr = FT_Get_Postscript_Name(face))
			psname = ptr;
		FT_Done_Face(face);
	}
	return psname;
}


int FontEngine::getUnitsPerEM () const {
	return _currentFace ? _currentFace->units_per_EM : 0;
}


/** Returns the ascender of the current font in font units.
 *  The (usually) positive value denotes the maximum height
 *  (extent above the baseline) of the font. */
int FontEngine::getAscender () const {
	return _currentFace ? _currentFace->ascender : 0;
}


/** Returns the descender of the current font in font units.
 *  The (usually) positive value denotes the maximum depth
 *  (extent below the baseline) of the font. */
int FontEngine::getDescender () const {
	return _currentFace ? -_currentFace->descender : 0;
}


int FontEngine::getHAdvance () const {
	if (_currentFace) {
		auto table = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(_currentFace, ft_sfnt_os2));
		return table ? table->xAvgCharWidth : 0;
	}
	return 0;
}


/** Returns the horizontal advance width of a given character in font units. */
int FontEngine::getHAdvance (const Character &c) const {
	if (_currentFace) {
		FT_Fixed adv=0;
		FT_Get_Advance(_currentFace, charIndex(c), FT_LOAD_NO_SCALE, &adv);
		return adv;
	}
	return 0;
}


/** Returns the vertical advance width of a given character in font units. */
int FontEngine::getVAdvance (const Character &c) const {
	if (_currentFace) {
		FT_Fixed adv=0;
		auto flags = FT_LOAD_NO_SCALE;
		if (FT_HAS_VERTICAL(_currentFace))
			flags |= FT_LOAD_VERTICAL_LAYOUT;
		FT_Get_Advance(_currentFace, charIndex(c), flags, &adv);
		return adv;
	}
	return 0;
}


int FontEngine::getWidth (const Character &c) const {
	if (_currentFace) {
		if (FT_Load_Glyph(_currentFace, charIndex(c), FT_LOAD_NO_SCALE) == 0)
			return _currentFace->glyph->metrics.width;
	}
	return 0;
}


int FontEngine::getHeight (const Character &c) const {
	if (_currentFace) {
		if (FT_Load_Glyph(_currentFace, charIndex(c), FT_LOAD_NO_SCALE) == 0)
			return _currentFace->glyph->metrics.horiBearingY;
	}
	return 0;
}


int FontEngine::getDepth (const Character &c) const {
	if (_currentFace) {
		if (FT_Load_Glyph(_currentFace, charIndex(c), FT_LOAD_NO_SCALE) == 0)
			return _currentFace->glyph->metrics.height - _currentFace->glyph->metrics.horiBearingY;
	}
	return 0;
}


int FontEngine::getCharIndexByGlyphName(const char *name) const {
	return _currentFace ? int(FT_Get_Name_Index(_currentFace, name)) : 0;
}


int FontEngine::charIndex (const Character &c) const {
	if (!_currentFace || !_currentFace->charmap)
		return c.type() == Character::NAME ? 0 : c.number();
	switch (c.type()) {
		case Character::CHRCODE:
			return FT_Get_Char_Index(_currentFace, (FT_ULong)c.number());
		case Character::NAME:
			return FT_Get_Name_Index(_currentFace, const_cast<FT_String*>(c.name()));
		default:
			return c.number();
	}
}


/** Returns the number of glyphs present in the current font face. */
int FontEngine::getNumGlyphs () const {
	return _currentFace ? _currentFace->num_glyphs : 0;
}


/** Returns the glyph name for a given character code.
 * @param[in] c char code
 * @return glyph name */
string FontEngine::getGlyphName (const Character &c) const {
	string ret;
	if (c.type() == Character::NAME)
		ret = c.name();
	else if (_currentFace && FT_HAS_GLYPH_NAMES(_currentFace)) {
		char buf[256];
		FT_Get_Glyph_Name(_currentFace, charIndex(c), buf, 256);
		ret = string(buf);
	}
	return ret;
}


vector<int> FontEngine::getPanose () const {
	vector<int> panose(10);
	if (_currentFace) {
		auto table = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(_currentFace, ft_sfnt_os2));
		if (table)
			for (int i=0; i < 10; i++)
				panose[i] = table->panose[i];
	}
	return panose;
}


int FontEngine::getCharMapIDs (vector<CharMapID> &charmapIDs) const {
	charmapIDs.clear();
	if (_currentFace) {
		for (int i=0; i < _currentFace->num_charmaps; i++) {
			FT_CharMap charmap = _currentFace->charmaps[i];
			charmapIDs.emplace_back(uint8_t(charmap->platform_id), uint8_t(charmap->encoding_id));
		}
	}
	return charmapIDs.size();
}


CharMapID FontEngine::setUnicodeCharMap () const {
	if (_currentFace && FT_Select_Charmap(_currentFace, FT_ENCODING_UNICODE) == 0)
		return {uint8_t(_currentFace->charmap->platform_id), uint8_t(_currentFace->charmap->encoding_id)};
	return {};
}


CharMapID FontEngine::setCustomCharMap () const {
	if (_currentFace && FT_Select_Charmap(_currentFace, FT_ENCODING_ADOBE_CUSTOM) == 0)
		return {uint8_t(_currentFace->charmap->platform_id), uint8_t(_currentFace->charmap->encoding_id)};
	return {};
}


// handle API change in freetype version 2.2.1
#if FREETYPE_MAJOR > 2 || (FREETYPE_MAJOR == 2 && (FREETYPE_MINOR > 2 || (FREETYPE_MINOR == 2 && FREETYPE_PATCH >= 1)))
	using FTVectorPtr = const FT_Vector*;
#else
	using FTVectorPtr = FT_Vector*;
#endif


// Callback functions used by trace_outline/FT_Outline_Decompose
static int moveto (FTVectorPtr to, void *user) {
	auto glyph = static_cast<Glyph*>(user);
	glyph->moveto(to->x, to->y);
	return 0;
}


static int lineto (FTVectorPtr to, void *user) {
	auto glyph = static_cast<Glyph*>(user);
	glyph->lineto(to->x, to->y);
	return 0;
}


static int quadto (FTVectorPtr control, FTVectorPtr to, void *user) {
	auto glyph = static_cast<Glyph*>(user);
	glyph->quadto(control->x, control->y, to->x, to->y);
	return 0;
}


static int cubicto (FTVectorPtr control1, FTVectorPtr control2, FTVectorPtr to, void *user) {
	auto glyph = static_cast<Glyph*>(user);
	glyph->cubicto(control1->x, control1->y, control2->x, control2->y, to->x, to->y);
	return 0;
}


/** Traces the outline of a glyph by calling the corresponding "drawing" functions.
 *  Each glyph is composed of straight lines, quadratic or cubic Bézier curves.
 *  This function creates a Glyph object representing these graphics segments.
 *  @param[in] face FreeType object representing the font to scan
 *  @param[in] font corresponding Font object providing additional data
 *  @param[in] index index of the glyph to be traced
 *  @param[out] glyph resulting Glyph object containing the graphics segments
 *  @param[in] scale if true the current pt size will be considered otherwise the plain TrueType units are used.
 *  @return false on errors */
static bool trace_outline (FT_Face face, const Font *font, int index, Glyph &glyph, bool scale) {
	if (face) {
		if (FT_Load_Glyph(face, index, scale ? FT_LOAD_DEFAULT : FT_LOAD_NO_SCALE)) {
			Message::estream(true) << "can't load glyph " << int(index) << '\n';
			return false;
		}
		if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
			Message::estream(true) << "no outlines found in glyph " << int(index) << '\n';
			return false;
		}
		FT_Outline outline = face->glyph->outline;
		// apply style parameters if set
		if (font) {
			if (const FontStyle *style = font->style()) {
				FT_Matrix matrix = {to_16dot16(style->extend), to_16dot16(style->slant), 0, to_16dot16(1)};
				FT_Outline_Transform(&outline, &matrix);
				if (style->bold != 0)
					FT_Outline_Embolden(&outline, style->bold/font->scaledSize()*face->units_per_EM);
			}
		}
		constexpr FT_Outline_Funcs funcs = {moveto, lineto, quadto, cubicto, 0, 0};
		FT_Outline_Decompose(&outline, &funcs, &glyph);
		return true;
	}
	Message::wstream(true) << "can't trace outline (no font selected)\n";
	return false;
}


/** Traces the outline of a glyph by calling the corresponding "drawing" functions.
 *  Each glyph is composed of straight lines, quadratic or cubic Bézier curves.
 *  This function creates a Glyph object representing these graphics segments.
 *  @param[in] c the glyph of this character will be traced
 *  @param[out] glyph resulting Glyph object containing the graphics segments
 *  @param[in] scale if true the current pt size will be considered otherwise the plain TrueType units are used.
 *  @return false on errors */
bool FontEngine::traceOutline (const Character &c, Glyph &glyph, bool scale) const {
	return trace_outline(_currentFace, _currentFont, charIndex(c), glyph, scale);
}