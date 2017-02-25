/*************************************************************************
** FontCache.hpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef FONTCACHE_HPP
#define FONTCACHE_HPP

#include <string>
#include <map>
#include "Glyph.hpp"


class FontCache
{
	typedef std::map<int, Glyph> GlyphMap;

	public:
		struct FontInfo
		{
			std::string name;   // fontname
			uint16_t version;   // file format version
			uint32_t checksum;  // CRC32 checksum of file data
			uint32_t numchars;  // number of characters
			uint32_t numbytes;  // number of bytes
			uint32_t numcmds;   // number of path commands
		};

	public:
		FontCache () : _changed(false) {}
		~FontCache () {clear();}
		bool read (const char *fontname, const char *dir);
		bool read (const char *fontname, std::istream &is);
		bool write (const char *dir) const;
		bool write (const char *fontname, const char *dir) const;
		bool write (const char *fontname, std::ostream &os) const;
		const Glyph* getGlyph (int c) const;
		void setGlyph (int c, const Glyph &glyph);
		void clear ();
		const std::string& fontname () const {return _fontname;}

		static bool fontinfo (const char *dirname, std::vector<FontInfo> &infos, std::vector<std::string> &invalid);
		static bool fontinfo (std::istream &is, FontInfo &info);
		static void fontinfo (const char *dirname, std::ostream &os, bool purge=false);

	private:
		static const uint8_t FORMAT_VERSION;
		std::string _fontname;
		GlyphMap _glyphs;
		bool _changed;
};

#endif
