/*************************************************************************
** FontManager.h                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2016 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef DVISVGM_FONTMANAGER_H
#define DVISVGM_FONTMANAGER_H

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <stack>
#include <vector>
#include "Color.h"
#include "FontStyle.h"
#include "types.h"


struct FileFinder;
class Font;
class  VirtualFont;

/** This class provides methods for easy DVI font handling.
 *  DVI and VF files use local font numbers to reference fonts. For SVG output
 *  we need a single list with unique IDs of all physical fonts. Characters of
 *  virtual fonts are completely replaced by their DVI description so they don't
 *  appear anywhere in the output. */
class FontManager
{
	typedef std::map<UInt32,int> Num2IdMap;
	typedef std::map<std::string,int> Name2IdMap;
	typedef std::map<const VirtualFont*,Num2IdMap> VfNum2IdMap;
	typedef std::map<const VirtualFont*, UInt32> VfFirstFontMap;
	typedef std::stack<VirtualFont*> VfStack;

	public:
		~FontManager ();
		static FontManager& instance ();
		int registerFont (UInt32 fontnum, std::string fontname, UInt32 checksum, double dsize, double scale);
		int registerFont (UInt32 fontnum, std::string fname, double ptsize, const FontStyle &style, Color color);
		int registerFont (UInt32 fontnum, std::string fname, int fontIndex, double ptsize, const FontStyle &style, Color color);
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
		void assignVfChar (int c, std::vector<UInt8> *dvi);
		const std::vector<Font*>& getFonts () const {return _fonts;}
		std::ostream& write (std::ostream &os, Font *font=0, int level=0);

	protected:
		FontManager () {}

	private:
		std::vector<Font*> _fonts; ///< all registered Fonts
		Num2IdMap      _num2id;    ///< DVI font number -> fontID
		Name2IdMap     _name2id;   ///< fontname -> fontID
		VfNum2IdMap    _vfnum2id;
		VfStack        _vfStack;   ///< stack of currently processed virtual fonts
		VfFirstFontMap _vfFirstFontMap; ///< VF -> local font number of first font defined in VF
};

#endif
