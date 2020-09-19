/*************************************************************************
** FontManager.hpp                                                      **
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

#ifndef FONTMANAGER_HPP
#define FONTMANAGER_HPP

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <stack>
#include <unordered_map>
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
class FontManager
{
	using Num2IdMap = std::unordered_map<uint32_t,int>;
	using Name2IdMap = std::unordered_map<std::string,int>;
	using VfNum2IdMap = std::unordered_map<const VirtualFont*,Num2IdMap>;
	using VfFirstFontMap = std::unordered_map<const VirtualFont*,uint32_t>;
	using VfStack = std::stack<VirtualFont*>;

	public:
		static FontManager& instance ();
		int registerFont (uint32_t fontnum, const std::string &fontname, uint32_t checksum, double dsize, double scale);
		int registerFont (uint32_t fontnum, const std::string &fname, double ptsize, const FontStyle &style, Color color);
		int registerFont (uint32_t fontnum, std::string fname, int fontIndex, double ptsize, const FontStyle &style, Color color);
		Font* getFont (int n) const;
		Font* getFont (const std::string &name) const;
		Font* getFontById (int id) const;
		const VirtualFont* getVF () const;
		int fontID (int n) const;
		int fontID (const Font *font) const;
		int fontID (const std::string &name) const;
		int fontnum (int id) const;
		int vfFirstFontNum (const VirtualFont *vf) const;
		void enterVF (VirtualFont *vf);
		void leaveVF ();
		void assignVFChar (int c, std::vector<uint8_t> &&dvi);
		std::ostream& write (std::ostream &os, Font *font=nullptr, int level=0);

	protected:
		FontManager () =default;

	private:
		std::vector<std::unique_ptr<Font>> _fonts; ///< all registered Fonts
		Num2IdMap      _num2id;    ///< DVI font number -> fontID
		Name2IdMap     _name2id;   ///< fontname -> fontID
		VfNum2IdMap    _vfnum2id;
		VfStack        _vfStack;   ///< stack of currently processed virtual fonts
		VfFirstFontMap _vfFirstFontMap; ///< VF -> local font number of first font defined in VF
};

#endif
