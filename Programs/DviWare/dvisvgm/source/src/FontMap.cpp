/*************************************************************************
** FontMap.cpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2021 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>
#include <vector>
#include "CMap.hpp"
#include "Directory.hpp"
#include "FileFinder.hpp"
#include "FontManager.hpp"
#include "FontMap.hpp"
#include "MapLine.hpp"
#include "Message.hpp"
#include "Subfont.hpp"
#include "utility.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;

/** Returns the singleton instance. */
FontMap& FontMap::instance() {
	static FontMap fontmap;
	return fontmap;
}


/** Reads and evaluates a single font map file.
 *  @param[in] fname name of map file to read
 *  @param[in] mode selects how to integrate the map file entries into the global map tree
 *  @return true if file could be opened */
bool FontMap::read (const string &fname, FontMap::Mode mode) {
#if defined(MIKTEX_WINDOWS)
        ifstream ifs(EXPATH_(fname));
#else
	ifstream ifs(fname);
#endif
	if (!ifs)
		return false;

	int line_number = 1;
	while (ifs) {
		int c = ifs.peek();
		if (c < 0 || strchr("\n&#%;*", c))  // comment or empty line?
			ifs.ignore(numeric_limits<int>::max(), '\n');
		else {
			try {
				MapLine mapline(ifs);
				apply(mapline, mode);
			}
			catch (const MapLineException &e) {
				Message::wstream(true) << fname << ", line " << line_number << ": " << e.what() << '\n';
			}
			catch (const SubfontException &e) {
				Message::wstream(true) << e.filename();
				if (e.lineno() > 0)
					Message::wstream(false) << ", line " << e.lineno();
				Message::wstream(false) << e.what() << '\n';
			}
		}
		line_number++;
	}
	return true;
}


bool FontMap::read (const string &fname, char modechar) {
	Mode mode;
	switch (modechar) {
		case '=': mode = Mode::REPLACE; break;
		case '-': mode = Mode::REMOVE; break;
		default : mode = Mode::APPEND;
	}
	return read(fname, mode);
}


/** Applies a mapline according to the given mode (append, remove, replace).
 *  @param[in] mapline the mapline to be applied
 *  @param[in] mode mode to use
 *  @return true in case of success */
bool FontMap::apply (const MapLine& mapline, FontMap::Mode mode) {
	switch (mode) {
		case Mode::APPEND:
			return append(mapline);
		case Mode::REMOVE:
			return remove(mapline);
		default:
			return replace(mapline);
	}
}


/** Applies a mapline according to the given mode (append, remove, replace).
 *  @param[in] mapline the mapline to be applied
 *  @param[in] modechar character that denotes the mode (+, -, or =)
 *  @return true in case of success */
bool FontMap::apply (const MapLine& mapline, char modechar) {
	Mode mode;
	switch (modechar) {
		case '=': mode = Mode::REPLACE; break;
		case '-': mode = Mode::REMOVE; break;
		default : mode = Mode::APPEND;
	}
	return apply(mapline, mode);
}


/** Reads and evaluates a sequence of map files. Each map file is looked up in the local
 *  directory and the TeX file tree.
 *  @param[in] fname_seq comma-separated list of map file names
 *  @return true if at least one of the given map files was found */
bool FontMap::read (const string &fname_seq) {
	bool found = false;
	size_t left=0;
	while (left < fname_seq.length()) {
		const char modechar = fname_seq[left];
		if (strchr("+-=", modechar))
			left++;
		string fname;
		size_t right = fname_seq.find(',', left);
		if (right != string::npos)
			fname = fname_seq.substr(left, right-left);
		else {
			fname = fname_seq.substr(left);
			right = fname_seq.length();
		}
		if (!fname.empty()) {
			if (!read(fname, modechar)) {
				if (const char *path = FileFinder::instance().lookup(fname, false))
					found = found || read(path, modechar);
				else
					Message::wstream(true) << "map file " << fname << " not found\n";
			}
		}
		left = right+1;
	}
	return found;
}


/** Appends given map line data to the font map if there is no entry for the corresponding
 *  font in the map yet.
 *  @param[in] mapline parsed font data
 *  @return true if data has been appended */
bool FontMap::append (const MapLine &mapline) {
	bool appended = false;
	if (!mapline.texname().empty()) {
		if (!mapline.fontfname().empty() || !mapline.encname().empty()) {
			vector<Subfont*> subfonts;
			if (mapline.sfd())
				subfonts = mapline.sfd()->subfonts();
			else
				subfonts.push_back(nullptr);
			for (Subfont *subfont : subfonts) {
				string fontname = mapline.texname()+(subfont ? subfont->id() : "");
				auto it = _entries.find(fontname);
				if (it == _entries.end()) {
					_entries.emplace(fontname, util::make_unique<Entry>(mapline, subfont));
					appended = true;
				}
			}
		}
	}
	return appended;
}


/** Replaces the map data of the given font.
 *  If the font is locked (because it's already in use) nothing happens.
 *  @param[in] mapline parsed font data
 *  @return true if data has been replaced */
bool FontMap::replace (const MapLine &mapline) {
	if (mapline.texname().empty())
		return false;
	if (mapline.fontfname().empty() && mapline.encname().empty())
		return remove(mapline);

	vector<Subfont*> subfonts;
	if (mapline.sfd())
		subfonts = mapline.sfd()->subfonts();
	else
		subfonts.push_back(nullptr);
	for (Subfont *subfont : subfonts) {
		string fontname = mapline.texname()+(subfont ? subfont->id() : "");
		auto it = _entries.find(fontname);
		if (it == _entries.end())
			_entries.emplace(fontname, util::make_unique<Entry>(mapline, subfont));
		else if (!it->second->locked)
			*it->second = Entry(mapline, subfont);
	}
	return true;
}


/** Removes the map entry of the given font.
 *  If the font is locked (because it's already in use) nothing happens.
 *  @param[in] mapline parsed font data
 *  @return true if entry has been removed */
bool FontMap::remove (const MapLine &mapline) {
	bool removed = false;
	if (!mapline.texname().empty()) {
		vector<Subfont*> subfonts;
		if (mapline.sfd())
			subfonts = mapline.sfd()->subfonts();
		else
			subfonts.push_back(nullptr);
		for (const Subfont *subfont : subfonts) {
			string fontname = mapline.texname()+(subfont ? subfont->id() : "");
			auto it = _entries.find(fontname);
			if (it != _entries.end() && !it->second->locked) {
				_entries.erase(it);
				removed = true;
			}
		}
	}
	return removed;
}


ostream& FontMap::write (ostream &os) const {
	for (const auto &entry : _entries)
		os << entry.first << " -> " << entry.second->fontname << " [" << entry.second->encname << "]\n";
	return os;
}


/** Reads and evaluates all map files in the given directory.
 *  @param[in] dirname path to directory containing the map files to be read */
void FontMap::readdir (const string &dirname) {
	Directory dir(dirname);
	while (const char *fname = dir.read(Directory::ET_FILE)) {
		if (strlen(fname) >= 4 && strcmp(fname+strlen(fname)-4, ".map") == 0) {
			string path = dirname + "/" + fname;
			read(path);
		}
	}
}


/** Returns name of font that is mapped to a given font.
 * @param[in] fontname name of font whose mapped name is retrieved
 * @returns name of mapped font */
const FontMap::Entry* FontMap::lookup (const string &fontname) const {
	auto it = _entries.find(fontname);
	if (it == _entries.end())
		return nullptr;
	return it->second.get();
}


/** Sets the lock flag for the given font in order to avoid changing the map data of this font.
 *  @param[in] fontname name of font to be locked */
void FontMap::lockFont (const string& fontname) {
	auto it = _entries.find(fontname);
	if (it != _entries.end())
		it->second->locked = true;
}


/** Removes all (unlocked) entries from the font map.
 *  @param[in] unlocked_only if true, only unlocked entries are removed */
void FontMap::clear (bool unlocked_only) {
	if (!unlocked_only)
		_entries.clear();
	else {
		auto it=_entries.begin();
		while (it != _entries.end()) {
			if (it->second->locked)
				++it;
			else
				it = _entries.erase(it);
		}
	}
}

/////////////////////////////////////////////////

FontMap::Entry::Entry (const MapLine &mapline, Subfont *sf)
	: fontname(mapline.fontfname()), encname(mapline.encname()), subfont(sf), fontindex(mapline.fontindex()),
	  locked(false), style(mapline.bold(), mapline.extend(), mapline.slant())
{
}
