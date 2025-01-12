/*************************************************************************
** FontManager.cpp                                                      **
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
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <set>
#include "CMap.hpp"
#include "Font.hpp"
#include "fonts/Base14Fonts.hpp"
#include "FontEngine.hpp"
#include "FontManager.hpp"
#include "FileFinder.hpp"
#include "FileSystem.hpp"
#include "Message.hpp"
#include "SVGTree.hpp"

using namespace std;


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
		auto it = _num2id.find(n);
		return (it == _num2id.end()) ? -1 : it->second;
	}
	auto vit = _vfnum2id.find(_vfStack.top());
	if (vit == _vfnum2id.end())
		return -1;
	const Num2IdMap &num2id = vit->second;
	auto it = num2id.find(n);
	return (it == num2id.end()) ? -1 : it->second;
}


/** Returns a unique ID that identifies the font. Not the font object but the
 *  font pointer is looked up to get the ID. Thus, two different pointers
 *  referencing different objects of the same font are mapped to different IDs.
 *  @param[in] font pointer to font object to be identified
 *  @return non-negative font ID if font was found, -1 otherwise */
int FontManager::fontID (const Font *font) const {
	for (size_t i=0; i < _fonts.size(); i++)
		if (_fonts[i].get() == font)
			return i;
	return -1;
}


/** Returns a unique ID that identifies the font.
 *  @param[in] name name of font to be identified, e.g. cmr10
 *  @return non-negative font ID if font was found, -1 otherwise */
int FontManager::fontID (const string &name) const {
	auto it = _name2id.find(name);
	if (it == _name2id.end())
		return -1;
	return it->second;
}


int FontManager::fontID (string name, double ptsize) const {
	std::replace(name.begin(), name.end(), '+', '-');
	for (auto it = _fonts.begin(); it != _fonts.end(); ++it) {
		if (auto nativeFont = font_cast<NativeFont*>(it->get())) {
			if (nativeFont->name() == name && nativeFont->scaledSize() == ptsize)
				return int(std::distance(_fonts.begin(), it));
		}
	}
	return -1;
}


int FontManager::fontnum (int id) const {
	if (id < 0 || size_t(id) > _fonts.size())
		return -1;
	if (_vfStack.empty()) {
		for (const auto &entry : _num2id)
			if (entry.second == id)
				return entry.first;
	}
	else {
		auto it = _vfnum2id.find(_vfStack.top());
		if (it == _vfnum2id.end())
			return -1;
		for (const auto &entry : it->second)
			if (entry.second == id)
				return entry.first;
	}
	return -1;
}


int FontManager::vfFirstFontNum (const VirtualFont *vf) const {
	auto it = _vfFirstFontNumMap.find(vf);
	return (it == _vfFirstFontNumMap.end()) ? -1 : (int) it->second;
}


Font* FontManager::vfFirstFont (const VirtualFont *vf) const {
	auto it = _vfFirstFontMap.find(vf);
	return (it == _vfFirstFontMap.end()) ? nullptr : it->second;
}


/** Returns a previously registered font.
 *  @param[in] n local font number, as used in DVI and VF files
 *  @return pointer to font if font was found, 0 otherwise */
Font* FontManager::getFont (int n) const {
	int id = fontID(n);
	return (id < 0) ? nullptr : _fonts[id].get();
}


Font* FontManager::getFont (const string &name) const {
	int id = fontID(name);
	if (id < 0)
		return nullptr;
	return _fonts[id].get();
}


Font* FontManager::getFont (const string &name, double ptsize) {
	int id = fontID(name, ptsize);
	if (id < 0)
		return nullptr;
	return _fonts[id].get();
}


Font* FontManager::getFontById (int id) const {
	if (id < 0 || size_t(id) >= _fonts.size())
		return nullptr;
	return _fonts[id].get();
}


/** Returns the current active virtual font. */
const VirtualFont* FontManager::getVF () const {
	return _vfStack.empty() ? nullptr : _vfStack.top();
}


static unique_ptr<Font> create_font (const string &filename, const string &fontname, int fontindex, uint32_t checksum, double dsize, double ssize) {
	string ext;
	if (const char *dot = strrchr(filename.c_str(), '.'))
		ext = util::tolower(dot+1);
	if (!ext.empty() && FileFinder::instance().lookup(filename)) {
		if (ext == "pfb")
			return PhysicalFont::create(fontname, checksum, dsize, ssize, PhysicalFont::Type::PFB);
		if (ext == "otf")
			return PhysicalFont::create(fontname, checksum, dsize, ssize, PhysicalFont::Type::OTF);
		if (ext == "ttf")
			return PhysicalFont::create(fontname, checksum, dsize, ssize, PhysicalFont::Type::TTF);
		if (ext == "ttc")
			return PhysicalFont::create(fontname, fontindex, checksum, dsize, ssize);
		if (ext == "vf")
			return VirtualFont::create(fontname, checksum, dsize, ssize);
		if (ext == "mf")
			return PhysicalFont::create(fontname, checksum, dsize, ssize, PhysicalFont::Type::MF);
	}
	return nullptr;
}


/** Registers a new font to be managed by the FontManager. If there is
 *  already a registered font assigned to number n, nothing happens.
 *  @param[in] fontnum local font number, as used in DVI and VF files
 *  @param[in] name TFM fontname given in DVI file, e.g. cmr10
 *  @param[in] checksum checksum to be compared with TFM checksum
 *  @param[in] dsize design size in PS point units
 *  @param[in] ssize scaled size in PS point units
 *  @return id of registered font */
int FontManager::registerFont (uint32_t fontnum, const string &name, uint32_t checksum, double dsize, double ssize) {
	int id = fontID(fontnum);
	if (id >= 0)
		return id;

	unique_ptr<Font> newfont;
	const int newid = _fonts.size();   // the new font gets this ID
	auto it = _name2id.find(name);
	if (it != _name2id.end()) {  // font with same name already registered?
		const auto &font = _fonts[it->second];
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
		if (filename.rfind('.') != string::npos)
			newfont = create_font(filename, name, fontindex, checksum, dsize, ssize);
		else {
			// try various font file formats if the given file has no extension
			const char *exts[] = {"pfb", "otf", "ttc", "ttf", "vf", "mf", nullptr};
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
			newfont = util::make_unique<EmptyFont>(name);
			// print warning message about missing font file (only once for each filename)
			static set<string> missing_fonts;
			if (missing_fonts.find(filename) == missing_fonts.end()) {
				if (filename.rfind('.') == string::npos)
					Message::wstream(true) << "no font file found for '" << filename << "'\n";
				else
					Message::wstream(true) << "font file '" << filename << "' not found\n";
				missing_fonts.insert(filename);
			}
		}
		_name2id.emplace(name, newid);
	}
	_fonts.push_back(std::move(newfont));
	if (_vfStack.empty())  // register font referenced in dvi file?
		_num2id[fontnum] = newid;
	else {  // register font referenced in vf file
		const VirtualFont *vf = _vfStack.top();
		_vfnum2id[vf][fontnum] = newid;
		_vfFirstFontNumMap.emplace(vf, fontnum);
		_vfFirstFontMap.emplace(vf, _fonts.back().get());
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
int FontManager::registerFont (uint32_t fontnum, const string &filename, double ptsize, const FontStyle &style, Color color) {
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
int FontManager::registerFont (uint32_t fontnum, const string &filename, int fontIndex, double ptsize, const FontStyle &style, Color color) {
	int id = fontID(fontnum);
	if (id >= 0)
		return id;

	string fontname = NativeFont::uniqueName(filename, style);
	const char *path = filename.c_str();
	unique_ptr<Font> newfont;
	const int newid = _fonts.size();   // the new font gets this ID
	auto it = _name2id.find(fontname);
	if (it != _name2id.end()) {  // font with same name already registered?
		if (auto font = font_cast<NativeFont*>(_fonts[it->second].get()))
			newfont = font->clone(ptsize, style, color);
	}
	else {
		if (!FileSystem::exists(path)) {
			const char *fontFormats[] = {nullptr, "otf", "ttf"};
			for (const char *format : fontFormats) {
				if ((path = FileFinder::instance().lookup(filename, format, false)) != nullptr)
					break;
			}
		}
		if (path) {
			newfont.reset(new NativeFontImpl(path, fontIndex, ptsize, style, color));
			newfont->findAndAssignBaseFontMap();
		}
		if (!newfont) {
			// create dummy font as a placeholder if the proper font is not available
			newfont = util::make_unique<EmptyFont>(filename);
			// print warning message about missing font file (only once for each filename)
			static set<string> missing_fonts;
			if (missing_fonts.find(filename) == missing_fonts.end()) {
				Message::wstream(true) << "font file '" << filename << "' not found\n";
				missing_fonts.insert(filename);
			}
		}
		_name2id.emplace(fontname, newid);
	}
	_fonts.push_back(std::move(newfont));
	_num2id[fontnum] = newid;
	return newid;
}


/** Registers a native font that is referenced by its name instead of a DVI font number.
 *  @param[in] fname filename/path of the font file
 *  @param[in] fontname font name used if the font file doesn't provide one
 *  @param[in] ptsize font size in PS points
 *  return global ID assigned to the font */
int FontManager::registerFont (const string &fname, string fontname, double ptsize) {
	if (fname.empty())
		return -1;
	if (fname.size() > 6 && fname.substr(0,6) == "sys://") {
		fontname = fname.substr(6);
		if (!find_base14_font(fontname))
			return -1;
	}
	else if (!FileSystem::exists(fname) || (fontname.empty() && (fontname = FontEngine::instance().getPSName(fname)).empty()))
		return -1;
	int id = fontID(fontname, ptsize);
	if (id >= 0)
		return id;
	unique_ptr<NativeFont> nativeFont;
	id = fontID(fontname);
	if (id < 0) {
		nativeFont = util::make_unique<NativeFontImpl>(fname, fontname, ptsize);
		_name2id.emplace(std::move(fontname), int(_fonts.size()));
	}
	else {
		auto *nf = font_cast<NativeFont*>(getFontById(id));
		nativeFont = unique_ptr<NativeFont>(nf->clone(ptsize, FontStyle(), Color::BLACK));
	}
	id = int(_fonts.size());
	_fonts.push_back(std::move(nativeFont));
	return id;
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
 * @param[in] dvi DVI commands that describe character c */
void FontManager::assignVFChar (int c, vector<char> &&dvi) {
	if (!_vfStack.empty())
		_vfStack.top()->assignChar(c, std::move(dvi));
}


void FontManager::addUsedChar (const Font &font, int c) {
	_usedChars[font.uniqueFont()].insert(c);
	if (!SVGTree::USE_FONTS)
		_usedChars[&font].insert(c);
	_usedFonts.insert(&font);
}


void FontManager::resetUsedChars () {
	_usedChars.clear();
	_usedFonts.clear();
}


ostream& FontManager::write (ostream &os, Font *font, int level) {
#if 0
	if (font) {
		int id = -1;
		for (int i=0; i < _fonts.size() && id < 0; i++)
			if (_fonts[i] == font)
				id = i;

		VirtualFont *vf = font_cast<VirtualFont*>(font);
		for (int j=0; j < level+1; j++)
			os << "  ";
		os << "id " << id
			<< " fontnum " << fontnum(id) << " "
			<< (vf ? "VF" : "PF") << " "
			<< font->name()
			<< endl;

		if (vf) {
			enterVF(vf);
			const Num2IdMap &num2id = _vfnum2id.find(vf)->second;
			for (const auto &entry : num2id) {
				Font *font = _fonts[entry.second];
				write(os, font, level+1);
			}
			leaveVF();
		}
	}
	else {
		for (int i=0; i < _fonts.size(); i++)
			write(os, _fonts[i], level);
		os << endl;
	}
#endif
	return os;
}
