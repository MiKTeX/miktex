/*************************************************************************
** FontEngine.hpp                                                       **
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

#ifndef FONTENGINE_HPP
#define FONTENGINE_HPP

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CID_H
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Character.hpp"
#include "CharMapID.hpp"
#include "Glyph.hpp"
#include "RangeMap.hpp"

class Font;

/** This class provides methods to handle font files and font data.
 *  It's a wrapper for the Freetype font library. */
class FontEngine {
	public:
		~FontEngine ();
		static FontEngine& instance ();
		static std::string version ();
		bool setFont (const Font &font);
		bool isCIDFont() const;
		bool traceOutline (const Character &c, Glyph &glyph, bool scale=true) const;
		const char* getFamilyName () const;
		const char* getStyleName () const;
		int getUnitsPerEM () const;
		int getAscender () const;
		int getDescender () const;
		int getAdvance (int c) const;
		int getHAdvance () const;
		int getHAdvance (const Character &c) const;
		int getVAdvance (const Character &c) const;
		int getFirstChar () const;
		int getNextChar () const;
		int getCharMapIDs (std::vector<CharMapID> &charmapIDs) const;
		int getNumGlyphs () const;
		CharMapID setUnicodeCharMap ();
		CharMapID setCustomCharMap ();
		std::vector<int> getPanose () const;
		std::string getGlyphName (const Character &c) const;
		int getCharByGlyphName (const char *name) const;
		bool setCharMap (const CharMapID &charMapID);
		void buildCharMap (RangeMap &charmap);
		std::unique_ptr<const RangeMap> createCustomToUnicodeMap ();

	protected:
		FontEngine ();
		bool setFont (const std::string &fname, int fontindex, const CharMapID &charmapID);
		int charIndex (const Character &c) const;

	private:
		mutable unsigned int _currentChar=0, _currentGlyphIndex=0;
		FT_Face _currentFace = nullptr;
		FT_Library _library;
		const Font *_currentFont = nullptr;
};

#endif
