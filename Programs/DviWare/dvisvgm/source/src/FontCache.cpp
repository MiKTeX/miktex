/*************************************************************************
** FontCache.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <iomanip>
#include <sstream>
#include "CRC32.hpp"
#include "FileSystem.hpp"
#include "FontCache.hpp"
#include "Glyph.hpp"
#include "Pair.hpp"
#include "StreamReader.hpp"
#include "StreamWriter.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;

const uint8_t FontCache::FORMAT_VERSION = 5;


static Pair32 read_pair (int bytes, StreamReader &sr) {
	int32_t x = sr.readSigned(bytes);
	int32_t y = sr.readSigned(bytes);
	return Pair32(x, y);
}


/** Removes all data from the cache. This does not affect the cache files. */
void FontCache::clear () {
	_glyphs.clear();
	_fontname.clear();
}


/** Assigns glyph data to a character and adds it to the cache.
 *  @param[in] c character code
 *  @param[in] glyph font glyph data */
void FontCache::setGlyph (int c, const Glyph &glyph) {
	_glyphs[c] = glyph;
	_changed = true;
}


/** Returns the corresponding glyph data of a given character of the current font.
 *  @param[in] c character code
 *  @return font glyph data (0 if no matching data was found) */
const Glyph* FontCache::getGlyph (int c) const {
	auto it = _glyphs.find(c);
	return (it != _glyphs.end()) ? &it->second : 0;
}


/** Writes the current cache data to a file (only if anything changed after
 *  the last call of read()).
 *  @param[in] fontname name of current font
 *  @param[in] dir directory where the cache file should go
 *  @return true if writing was successful */
bool FontCache::write (const string &fontname, const string &dir) const {
	if (!_changed)
		return true;

	if (!fontname.empty()) {
		string dirstr = dir.empty() ? FileSystem::getcwd() : dir;
		ostringstream oss;
		oss << dirstr << '/' << fontname << ".fgd";
#if defined(MIKTEX_WINDOWS)
                ofstream ofs(UW_(oss.str()), ios::binary);
#else
		ofstream ofs(oss.str(), ios::binary);
#endif
		return write(fontname, ofs);
	}
	return false;
}


bool FontCache::write (const string &dir) const {
	return _fontname.empty() ? false : write(_fontname, dir);
}


/** Returns the minimal number of bytes needed to store the given value. */
static int max_int_size (int32_t value) {
	int32_t limit = 0x7f;
	for (int i=1; i <= 4; i++) {
		if ((value < 0  && -value <= limit+1) || (value >= 0 && value <= limit))
			return i;
		limit = (limit << 8) | 0xff;
	}
	return 4;
}


/** Returns the minimal number of bytes needed to store the biggest
 *  pair component of the given vector. */
static int max_int_size (const Pair<int32_t> *pairs, size_t n) {
	int ret=0;
	for (size_t i=0; i < n; i++) {
		ret = max(ret, max_int_size(pairs[i].x()));
		ret = max(ret, max_int_size(pairs[i].y()));
	}
	return ret;
}


/** Writes the current cache data to a stream (only if anything changed after
 *  the last call of read()).
 *  @param[in] fontname name of current font
 *  @param[in] os output stream
 *  @return true if writing was successful */
bool FontCache::write (const string &fontname, ostream &os) const {
	if (!_changed)
		return true;
	if (!os)
		return false;

	StreamWriter sw(os);
	CRC32 crc32;

	struct WriteActions : Glyph::Actions {
		WriteActions (StreamWriter &sw, CRC32 &crc32) : _sw(sw), _crc32(crc32) {}

		void draw (char cmd, const Glyph::Point *points, int n) override {
			int bytes = max_int_size(points, n);
			int cmdchar = (bytes << 5) | (cmd - 'A');
			_sw.writeUnsigned(cmdchar, 1, _crc32);
			for (int i=0; i < n; i++) {
				_sw.writeSigned(points[i].x(), bytes, _crc32);
				_sw.writeSigned(points[i].y(), bytes, _crc32);
			}
		}
		StreamWriter &_sw;
		CRC32 &_crc32;
	} actions(sw, crc32);

	sw.writeUnsigned(FORMAT_VERSION, 1, crc32);
	sw.writeUnsigned(0, 4);  // space for checksum
	sw.writeString(fontname, crc32, true);
	sw.writeUnsigned(_glyphs.size(), 4, crc32);
	for (const auto &charglyphpair : _glyphs) {
		const Glyph &glyph = charglyphpair.second;
		sw.writeUnsigned(charglyphpair.first, 4, crc32);
		sw.writeUnsigned(glyph.size(), 2, crc32);
		glyph.iterate(actions, false);
	}
	os.seekp(1);
	sw.writeUnsigned(crc32.get(), 4);  // insert CRC32 checksum
	os.seekp(0, ios::end);
	return true;
}


/** Reads font glyph information from a file.
 *  @param[in] fontname name of font data to read
 *  @param[in] dir directory where the cache files are located
 *  @return true if reading was successful */
bool FontCache::read (const string &fontname, const string &dir) {
	if (fontname.empty())
		return false;
	if (_fontname == fontname)
		return true;
	clear();
	string dirstr = dir.empty() ? FileSystem::getcwd() : dir;
	ostringstream oss;
	oss << dirstr << '/' << fontname << ".fgd";
#if defined(MIKTEX_WINDOWS)
        ifstream ifs(UW_(oss.str()), ios::binary);
#else
	ifstream ifs(oss.str(), ios::binary);
#endif
	return read(fontname, ifs);
}


/** Reads font glyph information from a stream.
 *  @param[in] fontname name of font data to read
 *  @param[in] is input stream to read the glyph data from
 *  @return true if reading was successful */
bool FontCache::read (const string &fontname, istream &is) {
	if (_fontname == fontname)
		return true;
	clear();
	_fontname = fontname;
	if (!is)
		return false;

	StreamReader sr(is);
	CRC32 crc32;
	if (sr.readUnsigned(1, crc32) != FORMAT_VERSION)
		return false;

	uint32_t crc32_cmp = sr.readUnsigned(4);
	crc32.update(is);
	if (crc32.get() != crc32_cmp)
		return false;

	is.clear();
	is.seekg(5);  // continue reading after checksum

	string fname = sr.readString();
	if (fname != fontname)
		return false;

	uint32_t num_glyphs = sr.readUnsigned(4);
	while (num_glyphs-- > 0) {
		uint32_t c = sr.readUnsigned(4);  // character code
		uint16_t s = sr.readUnsigned(2);  // number of path commands
		Glyph &glyph = _glyphs[c];
		while (s-- > 0) {
			uint8_t cmdval = sr.readUnsigned(1);
			uint8_t cmdchar = (cmdval & 0x1f) + 'A';
			int bytes = cmdval >> 5;
			switch (cmdchar) {
				case 'C': {
					Pair32 p1 = read_pair(bytes, sr);
					Pair32 p2 = read_pair(bytes, sr);
					Pair32 p3 = read_pair(bytes, sr);
					glyph.cubicto(p1, p2, p3);
					break;
				}
				case 'L':
					glyph.lineto(read_pair(bytes, sr));
					break;
				case 'M':
					glyph.moveto(read_pair(bytes, sr));
					break;
				case 'Q': {
					Pair32 p1 = read_pair(bytes, sr);
					Pair32 p2 = read_pair(bytes, sr);
					glyph.conicto(p1, p2);
					break;
				}
				case 'Z':
					glyph.closepath();
			}
		}
	}
	_changed = false;
	return true;
}


/** Collects font cache information.
 *  @param[in]  dirname path to font cache directory
 *  @param[out] infos the collected font information
 *  @param[out] invalid names of outdated/corrupted cache files
 *  @return true on success */
bool FontCache::fontinfo (const string &dirname, vector<FontInfo> &infos, vector<string> &invalid) {
	infos.clear();
	invalid.clear();
	if (!dirname.empty()) {
		vector<string> fnames;
		FileSystem::collect(dirname, fnames);
		for (const string &fname : fnames) {
			if (fname[0] == 'f' && fname.length() > 5 && fname.substr(fname.length()-4) == ".fgd") {
				FontInfo info;
				string path = dirname+"/"+(fname.substr(1));
#if defined(MIKTEX_WINDOWS)
                                ifstream ifs(UW_(path), ios::binary);
#else
				ifstream ifs(path, ios::binary);
#endif
				if (fontinfo(ifs, info))
					infos.emplace_back(move(info));
				else
					invalid.emplace_back(fname.substr(1));
			}
		}
	}
	return !infos.empty();
}


/** Collects font cache information of a single font.
 *  @param[in]  is input stream of the cache file
 *  @param[out] info the collected data
 *  @return true if data could be read, false if cache file is unavailable, outdated, or corrupted */
bool FontCache::fontinfo (std::istream &is, FontInfo &info) {
	info.name.clear();
	info.numchars = info.numbytes = info.numcmds = 0;
	if (is) {
		is.clear();
		is.seekg(0);
		try {
			StreamReader sr(is);
			CRC32 crc32;
			if ((info.version = sr.readUnsigned(1, crc32)) != FORMAT_VERSION)
				return false;

			info.checksum = sr.readUnsigned(4);
			crc32.update(is);
			if (crc32.get() != info.checksum)
				return false;

			is.clear();
			is.seekg(5);  // continue reading after checksum

			info.name = sr.readString();
			info.numchars = sr.readUnsigned(4);
			for (uint32_t i=0; i < info.numchars; i++) {
				sr.readUnsigned(4);  // character code
				uint16_t s = sr.readUnsigned(2);  // number of path commands
				while (s-- > 0) {
					uint8_t cmdval = sr.readUnsigned(1);
					uint8_t cmdchar = (cmdval & 0x1f) + 'A';
					int bytes = cmdval >> 5;
					int bc = 0;
					switch (cmdchar) {
						case 'C': bc = 6*bytes; break;
						case 'H':
						case 'L':
						case 'M':
						case 'T':
						case 'V': bc = 2*bytes; break;
						case 'Q':
						case 'S': bc = 4*bytes; break;
						case 'Z': break;
						default : return false;
					}
					info.numbytes += bc+1; // command length + command
					info.numcmds++;
					is.seekg(bc, ios::cur);
				}
				info.numbytes += 6; // number of path commands + char code
			}
			info.numbytes += 6+info.name.length(); // version + 0-byte + fontname + number of chars
		}
		catch (StreamReaderException &e) {
			return false;
		}
	}
	return true;
}


/** Collects font cache information and write it to a stream.
 *  @param[in] dirname path to font cache directory
 *  @param[in] os output is written to this stream
 *  @param[in] purge if true, outdated and corrupted cache files are removed */
void FontCache::fontinfo (const string &dirname, ostream &os, bool purge) {
	if (!dirname.empty()) {
		ios::fmtflags osflags(os.flags());
		vector<FontInfo> infos;
		vector<string> invalid_files;
		if (!fontinfo(dirname, infos, invalid_files))
			os << "cache is empty\n";
		else {
			os << "cache format version " << infos[0].version << endl;
			map<string, const FontInfo*> sortmap;
			for (const FontInfo &info : infos)
				sortmap[info.name] = &info;
			for (const auto &strinfopair : sortmap) {
				os	<< dec << setfill(' ') << left
					<< setw(10) << left  << strinfopair.second->name
					<< setw(5)  << right << strinfopair.second->numchars << " glyph" << (strinfopair.second->numchars == 1 ? ' ':'s')
					<< setw(10) << right << strinfopair.second->numcmds  << " cmd"   << (strinfopair.second->numcmds == 1 ? ' ':'s')
					<< setw(12) << right << strinfopair.second->numbytes << " byte"  << (strinfopair.second->numbytes == 1 ? ' ':'s')
					<< setw(6) << "crc:" << setw(8) << hex << right << setfill('0') << strinfopair.second->checksum
					<< endl;
			}
		}
		if (purge) {
			for (const string &str : invalid_files) {
				string path=dirname+"/"+str;
				if (FileSystem::remove(path))
					os << "invalid cache file " << str << " removed\n";
			}
		}
		os.flags(osflags);  // restore format flags
	}
}
