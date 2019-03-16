/*************************************************************************
** FontEngine.cpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2019 Martin Gieseking <martin.gieseking@uos.de>   **
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
#  include <config.h>
#endif
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
#include "Message.hpp"
#include "utility.hpp"

using namespace std;


/** Converts a floating point value to a 16.16 fixed point value. */
static inline FT_Fixed to_16dot16 (double val) {
	return static_cast<FT_Fixed>(val*65536.0 + 0.5);
}


/** Converts an integer to a 16.16 fixed point value. */
static inline FT_Fixed to_16dot16 (int val) {
	return static_cast<FT_Fixed>(val) << 16;
}


///////////////////////////////////////////////////////////////////////////


FontEngine::FontEngine () : _currentFace(0), _currentFont(0)
{
	_currentChar = _currentGlyphIndex = 0;
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
 * @return true on success */
bool FontEngine::setFont (const string &fname, int fontindex, const CharMapID &charMapID) {
	if (_currentFace && FT_Done_Face(_currentFace))
		Message::estream(true) << "failed to release font\n";
	if (FT_New_Face(_library, fname.c_str(), fontindex, &_currentFace)) {
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
		const PhysicalFont *pf = dynamic_cast<const PhysicalFont*>(&font);
		if (setFont(path, font.fontIndex(), pf ? pf->getCharMapID() : CharMapID())) {
			_currentFont = &font;
			return true;
		}
	}
	return false;
}


bool FontEngine::isCIDFont() const {
	FT_Bool cid_keyed;
	return FT_Get_CID_Is_Internally_CID_Keyed(_currentFace, &cid_keyed) == 0 && cid_keyed;
}


bool FontEngine::setCharMap (const CharMapID &charMapID) {
	for (int i=0; i < _currentFace->num_charmaps; i++) {
		FT_CharMap ft_cmap = _currentFace->charmaps[i];
		if (ft_cmap->platform_id == charMapID.platform_id && ft_cmap->encoding_id == charMapID.encoding_id) {
			FT_Set_Charmap(_currentFace, ft_cmap);
			return true;
		}
	}
	return false;
}


/** Returns a character map that maps from character indexes to character codes
 *  of the current encoding.
 *  @param[out] charmap the resulting charmap */
void FontEngine::buildCharMap (RangeMap &charmap) {
	charmap.clear();
	FT_UInt glyph_index;
	uint32_t charcode = FT_Get_First_Char(_currentFace, &glyph_index);
	while (glyph_index) {
		charmap.addRange(glyph_index, glyph_index, charcode);
		charcode = FT_Get_Next_Char(_currentFace, charcode, &glyph_index);
	}
}


/** Creates a charmap that maps from the custom character encoding to unicode.
 *  @return pointer to charmap if it could be created, 0 otherwise */
unique_ptr<const RangeMap> FontEngine::createCustomToUnicodeMap () {
	FT_CharMap ftcharmap = _currentFace->charmap;
	if (FT_Select_Charmap(_currentFace, FT_ENCODING_ADOBE_CUSTOM) != 0)
		return nullptr;
	RangeMap index_to_source_chrcode;
	buildCharMap(index_to_source_chrcode);
	if (FT_Select_Charmap(_currentFace, FT_ENCODING_UNICODE) != 0)
		return nullptr;
	auto charmap = util::make_unique<RangeMap>();
	FT_UInt glyph_index;
	uint32_t unicode_point = FT_Get_First_Char(_currentFace, &glyph_index);
	while (glyph_index) {
		uint32_t custom_charcode = index_to_source_chrcode.valueAt(glyph_index);
		charmap->addRange(custom_charcode, custom_charcode, unicode_point);
		unicode_point = FT_Get_Next_Char(_currentFace, unicode_point, &glyph_index);
	}
	FT_Set_Charmap(_currentFace, ftcharmap);
	return std::move(charmap);
}


const char* FontEngine::getFamilyName () const {
	return _currentFace ? _currentFace->family_name : nullptr;
}


const char* FontEngine::getStyleName () const {
	return _currentFace ? _currentFace->style_name : nullptr;
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


int FontEngine::getAdvance (int c) const {
	if (_currentFace) {
		FT_Fixed adv=0;
		FT_Get_Advance(_currentFace, c, FT_LOAD_NO_SCALE, &adv);
		return adv;
	}
	return 0;
}


int FontEngine::getHAdvance () const {
	if (_currentFace) {
		TT_OS2 *table = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(_currentFace, ft_sfnt_os2));
		return table ? table->xAvgCharWidth : 0;
	}
	return 0;
}


int FontEngine::getHAdvance (const Character &c) const {
	if (_currentFace) {
		FT_Load_Glyph(_currentFace, charIndex(c), FT_LOAD_NO_SCALE);
		return _currentFace->glyph->metrics.horiAdvance;
	}
	return 0;
}


int FontEngine::getVAdvance (const Character &c) const {
	if (_currentFace) {
		FT_Load_Glyph(_currentFace, charIndex(c), FT_LOAD_NO_SCALE);
		if (FT_HAS_VERTICAL(_currentFace))
			return _currentFace->glyph->metrics.vertAdvance;
		return _currentFace->glyph->metrics.horiAdvance;
	}
	return 0;
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


/** Get first available character of the current font face. */
int FontEngine::getFirstChar () const {
	if (_currentFace)
		return _currentChar = FT_Get_First_Char(_currentFace, &_currentGlyphIndex);
	return 0;
}


/** Get the next available character of the current font face. */
int FontEngine::getNextChar () const {
	if (_currentFace && _currentGlyphIndex)
		return _currentChar = FT_Get_Next_Char(_currentFace, _currentChar, &_currentGlyphIndex);
	return getFirstChar();
}


/** Returns the number of glyphs present in the current font face. */
int FontEngine::getNumGlyphs () const {
	return _currentFace ? _currentFace->num_glyphs : 0;
}


/** Returns the glyph name for a given charater code.
 * @param[in] c char code
 * @return glyph name */
string FontEngine::getGlyphName (const Character &c) const {
	if (c.type() == Character::NAME)
		return c.name();

	if (_currentFace && FT_HAS_GLYPH_NAMES(_currentFace)) {
		char buf[256];
		FT_Get_Glyph_Name(_currentFace, charIndex(c), buf, 256);
		return string(buf);
	}
	return "";
}


vector<int> FontEngine::getPanose () const {
	vector<int> panose(10);
	if (_currentFace) {
		TT_OS2 *table = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(_currentFace, ft_sfnt_os2));
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
			charmapIDs.emplace_back(CharMapID(charmap->platform_id, charmap->encoding_id));
		}
	}
	return charmapIDs.size();
}


CharMapID FontEngine::setUnicodeCharMap () {
	if (_currentFace && FT_Select_Charmap(_currentFace, FT_ENCODING_UNICODE) == 0)
		return CharMapID(_currentFace->charmap->platform_id, _currentFace->charmap->encoding_id);
	return CharMapID();
}


CharMapID FontEngine::setCustomCharMap () {
	if (_currentFace && FT_Select_Charmap(_currentFace, FT_ENCODING_ADOBE_CUSTOM) == 0)
		return CharMapID(_currentFace->charmap->platform_id, _currentFace->charmap->encoding_id);
	return CharMapID();
}


// handle API change in freetype version 2.2.1
#if FREETYPE_MAJOR > 2 || (FREETYPE_MAJOR == 2 && (FREETYPE_MINOR > 2 || (FREETYPE_MINOR == 2 && FREETYPE_PATCH >= 1)))
	using FTVectorPtr = const FT_Vector*;
#else
	using FTVectorPtr = FT_Vector*;
#endif


// Callback functions used by trace_outline/FT_Outline_Decompose
static int moveto (FTVectorPtr to, void *user) {
	Glyph *glyph = static_cast<Glyph*>(user);
	glyph->moveto(to->x, to->y);
	return 0;
}


static int lineto (FTVectorPtr to, void *user) {
	Glyph *glyph = static_cast<Glyph*>(user);
	glyph->lineto(to->x, to->y);
	return 0;
}


static int conicto (FTVectorPtr control, FTVectorPtr to, void *user) {
	Glyph *glyph = static_cast<Glyph*>(user);
	glyph->conicto(control->x, control->y, to->x, to->y);
	return 0;
}


static int cubicto (FTVectorPtr control1, FTVectorPtr control2, FTVectorPtr to, void *user) {
	Glyph *glyph = static_cast<Glyph*>(user);
	glyph->cubicto(control1->x, control1->y, control2->x, control2->y, to->x, to->y);
	return 0;
}


/** Traces the outline of a glyph by calling the corresponding "drawing" functions.
 *  Each glyph is composed of straight lines, quadratic (conic) or cubic B�zier curves.
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
		if (const FontStyle *style = font->style()) {
			FT_Matrix matrix = {to_16dot16(style->extend), to_16dot16(style->slant), 0, to_16dot16(1)};
			FT_Outline_Transform(&outline, &matrix);
			if (style->bold != 0)
				FT_Outline_Embolden(&outline, style->bold/font->scaledSize()*face->units_per_EM);
		}
		const FT_Outline_Funcs funcs = {moveto, lineto, conicto, cubicto, 0, 0};
		FT_Outline_Decompose(&outline, &funcs, &glyph);
		return true;
	}
	Message::wstream(true) << "can't trace outline (no font selected)\n";
	return false;
}


/** Traces the outline of a glyph by calling the corresponding "drawing" functions.
 *  Each glyph is composed of straight lines, quadratic (conic) or cubic B�zier curves.
 *  This function creates a Glyph object representing these graphics segments.
 *  @param[in] c the glyph of this character will be traced
 *  @param[out] glyph resulting Glyph object containing the graphics segments
 *  @param[in] scale if true the current pt size will be considered otherwise the plain TrueType units are used.
 *  @return false on errors */
bool FontEngine::traceOutline (const Character &c, Glyph &glyph, bool scale) const {
	return trace_outline(_currentFace, _currentFont, charIndex(c), glyph, scale);
}
