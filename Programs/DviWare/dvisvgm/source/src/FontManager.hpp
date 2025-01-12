/*************************************************************************
** FontManager.hpp                                                      **
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

#ifndef FONTMANAGER_HPP
#define FONTMANAGER_HPP

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Color.hpp"
#include "FontStyle.hpp"


class FileFinder;
class Font;
class VirtualFont;

/** This class provides methods for easy DVI font handling.
 *  DVI and VF files use local font numbers to reference fonts. For SVG output
 *  we need a single list with unique IDs of all physical fonts. Characters of
 *  virtual fonts are completely replaced by their DVI description so they don't
 *  appear anywhere in the output. */
class FontManager {
	using CharMap = std::unordered_map<const Font*, std::set<int>>;
	using FontSet = std::unordered_set<const Font*>;
	using Num2IdMap = std::unordered_map<uint32_t, int>;
	using Name2IdMap = std::unordered_map<std::string, int>;
	using VfNum2IdMap = std::unordered_map<const VirtualFont*, Num2IdMap>;
	using VfFirstFontNumMap = std::unordered_map<const VirtualFont*, uint32_t>;
	using VfFirstFontMap = std::unordered_map<const VirtualFont*, Font*>;
	using VfStack = std::stack<VirtualFont*>;

	public:
		static FontManager& instance ();
		int registerFont (uint32_t fontnum, const std::string &fontname, uint32_t checksum, double dsize, double scale);
		int registerFont (uint32_t fontnum, const std::string &fname, double ptsize, const FontStyle &style, Color color);
		int registerFont (uint32_t fontnum, const std::string &fname, int fontIndex, double ptsize, const FontStyle &style, Color color);
		int registerFont (const std::string &fname, std::string fontname, double ptsize);
		Font* getFont (int n) const;
		Font* getFont (const std::string &name) const;
		Font* getFont (const std::string &name, double ptsize);
		Font* getFontById (int id) const;
		const VirtualFont* getVF () const;
		int fontID (int n) const;
		int fontID (const Font *font) const;
		int fontID (const std::string &name) const;
		int fontID (std::string name, double ptsize) const;
		int fontnum (int id) const;
		int vfFirstFontNum (const VirtualFont *vf) const;
		Font* vfFirstFont (const VirtualFont *vf) const;
		void enterVF (VirtualFont *vf);
		void leaveVF ();
		void assignVFChar (int c, std::vector<char> &&dvi);
		void addUsedChar (const Font &font, int c);
		void resetUsedChars ();
		CharMap& getUsedChars ()  {return _usedChars;}
		FontSet& getUsedFonts ()  {return _usedFonts;}
		std::ostream& write (std::ostream &os, Font *font=nullptr, int level=0);

	protected:
		FontManager () =default;

	private:
		std::vector<std::unique_ptr<Font>> _fonts; ///< all registered Fonts
		Num2IdMap      _num2id;    ///< DVI font number -> fontID
		Name2IdMap     _name2id;   ///< fontname -> fontID
		VfNum2IdMap    _vfnum2id;
		VfStack        _vfStack;   ///< stack of currently processed virtual fonts
		VfFirstFontNumMap _vfFirstFontNumMap; ///< VF -> local font number of first font defined in VF
		VfFirstFontMap _vfFirstFontMap;       ///< VF -> first font defined
		CharMap _usedChars;
		FontSet _usedFonts;
};

#endif
