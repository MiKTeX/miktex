/*************************************************************************
** FontMap.hpp                                                          **
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

#ifndef FONTMAP_HPP
#define FONTMAP_HPP

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include "FontStyle.hpp"

struct FontEncoding;
class MapLine;
class Subfont;

class FontMap {
	public:
		struct Entry {
			explicit Entry (const MapLine &mapline, Subfont *subfont=nullptr);
			Entry (const Entry &entry) =delete;
			Entry (Entry &&entry) =default;
			Entry& operator = (Entry &&entry) =default;
			std::string fontname; ///< target font name
			std::string encname;  ///< name of font encoding
			Subfont *subfont;
			int fontindex;        ///< index of font in multi-font file
			bool locked;
			FontStyle style;
		};

	public:
		enum class Mode {APPEND, REMOVE, REPLACE};

		static FontMap& instance ();
		bool read (const std::string &fname, Mode mode);
		bool read (const std::string &fname, char modechar);
		bool read (const std::string &fname_seq);
		void readdir (const std::string &dirname);
		bool apply (const MapLine &mapline, Mode mode);
		bool apply (const MapLine &mapline, char modechar);
		bool append (const MapLine &mapline);
		bool replace (const MapLine &mapline);
		bool remove (const MapLine &mapline);
		void lockFont (const std::string &fontname);
		void clear (bool unlocked_only=false);
		std::ostream& write (std::ostream &os) const;
		const Entry* lookup (const std::string &fontname) const;

	protected:
		FontMap () =default;

	private:
		std::unordered_map<std::string,std::unique_ptr<Entry>> _entries;
};

#endif
