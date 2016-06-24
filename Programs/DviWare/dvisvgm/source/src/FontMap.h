/*************************************************************************
** FontMap.h                                                            **
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

#ifndef DVISVGM_FONTMAP_H
#define DVISVGM_FONTMAP_H

#include <map>
#include <ostream>
#include <string>
#include "FontStyle.h"


struct FontEncoding;
class MapLine;
class Subfont;

class FontMap
{
	public:
		struct Entry
		{
			Entry (const MapLine &mapline, Subfont *subfont=0);
			std::string fontname; ///< target font name
			std::string encname;  ///< name of font encoding
			Subfont *subfont;
			int fontindex;        ///< index of font in multi-font file
			bool locked;
			FontStyle style;
		};

	protected:
		typedef std::map<std::string,Entry*>::iterator Iterator;
		typedef std::map<std::string,Entry*>::const_iterator ConstIterator;

	public:
		enum Mode {FM_APPEND, FM_REMOVE, FM_REPLACE};

		~FontMap ();
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
		const Entry* lookup(const std::string &fontname) const;

	protected:
		FontMap () {}

	private:
		std::map<std::string,Entry*> _entries;
};

#endif
