/*************************************************************************
** FontManager.cpp                                                      **
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

#include <config.h>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <set>
#include "CMap.h"
#include "Font.h"
#include "FontManager.h"
#include "FileFinder.h"
#include "FileSystem.h"
#include "Message.h"

using namespace std;


FontManager::~FontManager () {
	FORALL(_fonts, vector<Font*>::iterator, i)
		delete *i;
}


/** Returns the singleton instance */
FontManager& FontManager::instance () {
	static FontManager fm;
	return fm;
}


/** Returns a unique ID that identifies the font.
 *  @param[in] n local font number, as used in DVI and VF files
 *  @return non-negative font ID if font was found, -1 otherwise */
int FontManager::fontID (int n) const {
	if (_vfStack.empty()) {
		Num2IdMap::const_iterator it = _num2id.find(n);
		return (it == _num2id.end()) ? -1 : it->second;
	}
	VfNum2IdMap::const_iterator vit = _vfnum2id.find(_vfStack.top());
	if (vit == _vfnum2id.end())
		return -1;
	const Num2IdMap &num2id = vit->second;
	Num2IdMap::const_iterator it = num2id.find(n);
	return (it == num2id.end()) ? -1 : it->second;
}


/** Returns a unique ID that identifies the font. Not the font object but the
 *  font pointer is looked up to get the ID. Thus, two different pointers
 *  referencing different objects of the same font are mapped to different IDs.
 *  @param[in] font pointer to font object to be identified
 *  @return non-negative font ID if font was found, -1 otherwise */
int FontManager::fontID (const Font *font) const {
	for (size_t i=0; i < _fonts.size(); i++)
		if (_fonts[i] == font)
			return i;
	return -1;
}


/** Returns a unique ID that identifies the font.
 *  @param[in] name name of font to be identified, e.g. cmr10
 *  @return non-negative font ID if font was found, -1 otherwise */
int FontManager::fontID (const string &name) const {
	map<string,int>::const_iterator it = _name2id.find(name);
	if (it == _name2id.end())
		return -1;
	return it->second;
}


int FontManager::fontnum (int id) const {
	if (id < 0 || size_t(id) > _fonts.size())
		return -1;
	if (_vfStack.empty()) {
		FORALL(_num2id, Num2IdMap::const_iterator, i)
			if (i->second == id)
				return i->first;
	}
	else {
		VfNum2IdMap::const_iterator it = _vfnum2id.find(_vfStack.top());
		if (it == _vfnum2id.end())
			return -1;
		const Num2IdMap &num2id = it->second;
		FORALL(num2id, Num2IdMap::const_iterator, i)
			if (i->second == id)
				return i->first;
	}
	return -1;
}


int FontManager::vfFirstFontNum (const VirtualFont *vf) const {
	VfFirstFontMap::const_iterator it = _vfFirstFontMap.find(vf);
	return (it == _vfFirstFontMap.end()) ? -1 : (int) it->second;
}


/** Returns a previously registered font.
 *  @param[in] n local font number, as used in DVI and VF files
 *  @return pointer to font if font was found, 0 otherwise */
Font* FontManager::getFont (int n) const {
	int id = fontID(n);
	return (id < 0) ? 0 : _fonts[id];
}


Font* FontManager::getFont (const string &name) const {
	int id = fontID(name);
	if (id < 0)
		return 0;
	return _fonts[id];
}


Font* FontManager::getFontById (int id) const {
	if (id < 0 || size_t(id) >= _fonts.size())
		return 0;
	return _fonts[id];
}


/** Returns the current active virtual font. */
const VirtualFont* FontManager::getVF () const {
	return _vfStack.empty() ? 0 : _vfStack.top();
}


static Font* create_font (const string &filename, const string &fontname, int fontindex, UInt32 checksum, double dsize, double ssize) {
	string ext;
	if (const char *dot = strrchr(filename.c_str(), '.'))
		ext = dot+1;
	if (!ext.empty() && FileFinder::lookup(filename)) {
		if (ext == "pfb")
			return PhysicalFont::create(fontname, checksum, dsize, ssize, PhysicalFont::PFB);
		if (ext == "otf")
			return PhysicalFont::create(fontname, checksum, dsize, ssize, PhysicalFont::OTF);
		if (ext == "ttf")
			return PhysicalFont::create(fontname, checksum, dsize, ssize, PhysicalFont::TTF);
		if (ext == "ttc")
			return PhysicalFont::create(fontname, fontindex, checksum, dsize, ssize);
		if (ext == "vf")
			return VirtualFont::create(fontname, checksum, dsize, ssize);
		if (ext == "mf")
			return PhysicalFont::create(fontname, checksum, dsize, ssize, PhysicalFont::MF);
	}
	return 0;
}


/** Registers a new font to be managed by the FontManager. If there is
 *  already a registered font assigned to number n, nothing happens.
 *  @param[in] fontnum local font number, as used in DVI and VF files
 *  @param[in] name TFM fontname given in DVI file, e.g. cmr10
 *  @param[in] checksum checksum to be compared with TFM checksum
 *  @param[in] dsize design size in PS point units
 *  @param[in] ssize scaled size in PS point units
 *  @return id of registered font */
int FontManager::registerFont (UInt32 fontnum, string name, UInt32 checksum, double dsize, double ssize) {
	int id = fontID(fontnum);
	if (id >= 0)
		return id;

	Font *newfont = 0;
	const int newid = _fonts.size();   // the new font gets this ID
	Name2IdMap::iterator it = _name2id.find(name);
	if (it != _name2id.end()) {  // font with same name already registered?
		Font *font = _fonts[it->second];
		newfont = font->clone(dsize, ssize);
	}
	else {
		string filename = name;
		int fontindex = 0;
		const FontMap::Entry *map_entry = FontMap::instance().lookup(name);
		if (map_entry) {
			filename = map_entry->fontname;
			fontindex = map_entry->fontindex;
		}
		// try to find font file with the exact given name
		if (filename.rfind(".") != string::npos)
			newfont = create_font(filename, name, fontindex, checksum, dsize, ssize);
		else {
			// try various font file formats if the given file has no extension
			const char *exts[] = {"pfb", "otf", "ttc", "ttf", "vf", "mf", 0};
			for (const char **p = exts; *p && !newfont; ++p)
				newfont = create_font(filename+"."+*p, name, fontindex, checksum, dsize, ssize);
		}
		if (newfont) {
			if (!newfont->findAndAssignBaseFontMap())
				Message::wstream(true) << "no suitable encoding table found for font " << filename << "\n";
			if (!newfont->verifyChecksums())
				Message::wstream(true) << "checksum mismatch in font " << name << '\n';
		}
		else {
			// create dummy font as a placeholder if the proper font is not available
			newfont = new EmptyFont(name);
			if (filename.rfind(".") == string::npos)
				filename += ".mf";
			// print warning message about missing font file (only once for each filename)
			static set<string> missing_fonts;
			if (missing_fonts.find(filename) == missing_fonts.end()) {
				Message::wstream(true) << "font file '" << filename << "' not found\n";
				missing_fonts.insert(filename);
			}
		}
		_name2id[name] = newid;
	}
	_fonts.push_back(newfont);
	if (_vfStack.empty())  // register font referenced in dvi file?
		_num2id[fontnum] = newid;
	else {  // register font referenced in vf file
		VirtualFont *vf = const_cast<VirtualFont*>(_vfStack.top());
		_vfnum2id[vf][fontnum] = newid;
		if (_vfFirstFontMap.find(vf) == _vfFirstFontMap.end()) // first fontdef of VF?
			_vfFirstFontMap[vf] = fontnum;
	}
	return newid;
}


/** Registers a new native font to be managed by the FontManager. If there is
 *  already a registered font assigned to number n, nothing happens.
 *  @param[in] fontnum local font number used in DVI file
 *  @param[in] filename name/path of font file
 *  @param[in] ptsize font size in PS points
 *  @param[in] style font style parameters
 *  @param[in] color global font color
 *  @return global font id */
int FontManager::registerFont (UInt32 fontnum, string filename, double ptsize, const FontStyle &style, Color color) {
	return registerFont(fontnum, filename, 0, ptsize, style, color);
}


/** Registers a new native font to be managed by the FontManager. If there is
 *  already a registered font assigned to number n, nothing happens.
 *  @param[in] fontnum local font number used in DVI file
 *  @param[in] filename name/path of font file
 *  @param[in] fontIndex subfont index
 *  @param[in] ptsize font size in PS points
 *  @param[in] style font style parameters
 *  @param[in] color global font color
 *  @return global font id */
int FontManager::registerFont (UInt32 fontnum, string filename, int fontIndex, double ptsize, const FontStyle &style, Color color) {
	int id = fontID(fontnum);
	if (id >= 0)
		return id;

	if (!filename.empty() && filename[0] == '[' && filename[filename.size()-1] == ']')
		filename = filename.substr(1, filename.size()-2);
	string fontname = NativeFont::uniqueName(filename, style);
	const char *path = filename.c_str();
	Font *newfont=0;
	const int newid = _fonts.size();   // the new font gets this ID
	Name2IdMap::iterator it = _name2id.find(fontname);
	if (it != _name2id.end()) {  // font with same name already registered?
		if (NativeFont *font = dynamic_cast<NativeFont*>(_fonts[it->second]))
			newfont = font->clone(ptsize, style, color);
	}
	else {
		if (!FileSystem::exists(path))
			path = FileFinder::lookup(filename, false);
		if (path) {
			newfont = new NativeFontImpl(path, fontIndex, ptsize, style, color);
			newfont->findAndAssignBaseFontMap();
		}
		if (!newfont) {
			// create dummy font as a placeholder if the proper font is not available
			newfont = new EmptyFont(filename);
			// print warning message about missing font file (only once for each filename)
			static set<string> missing_fonts;
			if (missing_fonts.find(filename) == missing_fonts.end()) {
				Message::wstream(true) << "font file '" << filename << "' not found\n";
				missing_fonts.insert(filename);
			}
		}
		_name2id[fontname] = newid;
	}
	_fonts.push_back(newfont);
	_num2id[fontnum] = newid;
	return newid;
}


/** Enters a new virtual font frame.
 *  This method must be called before processing a VF character.
 *  @param[in] vf virtual font */
void FontManager::enterVF (VirtualFont *vf) {
	if (vf)
		_vfStack.push(vf);
}


/** Leaves a previously entered virtual font frame. */
void FontManager::leaveVF () {
	if (!_vfStack.empty())
		_vfStack.pop();
}


/** Assigns a sequence of DVI commands to a char code.
 * @param[in] c character code
 * @param[in] dvi points to vector with DVI commands */
void FontManager::assignVfChar (int c, vector<UInt8> *dvi) {
	if (!_vfStack.empty() && dvi)
		_vfStack.top()->assignChar(c, dvi);
}


ostream& FontManager::write (ostream &os, Font *font, int level) {
#if 0
	if (font) {
		int id = -1;
		for (int i=0; i < fonts.size() && id < 0; i++)
			if (fonts[i] == font)
				id = i;

		VirtualFont *vf = dynamic_cast<VirtualFont*>(font);
		for (int j=0; j < level+1; j++)
			os << "  ";
		os << "id " << id
			<< " fontnum " << fontnum(id) << " "
			<< (vf ? "VF" : "PF") << " "
			<< font->name()
			<< endl;

		if (vf) {
			enterVF(vf);
			const Num2IdMap &num2id = vfnum2id.find(vf)->second;
			FORALL(num2id, Num2IdMap::const_iterator, i) {
				Font *font = fonts[i->second];
				write(os, font, level+1);
			}
			leaveVF();
		}
	}
	else {
		for (int i=0; i < fonts.size(); i++)
			write(os, fonts[i], level);
		os << endl;
	}
#endif
	return os;
}

