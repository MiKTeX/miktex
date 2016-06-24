/*************************************************************************
** EncFile.h                                                            **
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

#ifndef DVISVGM_ENCFILE_H
#define DVISVGM_ENCFILE_H

#include <istream>
#include <map>
#include <string>
#include <vector>
#include "FontEncoding.h"
#include "types.h"


class EncFile : public NamedFontEncoding
{
	public:
		EncFile (const std::string &name);
		void read ();
		void read (std::istream &is);
		int size () const                  {return _table.size();}
		const char* name () const          {return _encname.c_str();}
		const char* charName (UInt32 c) const;
		Character decode (UInt32 c) const  {return Character(charName(c));}
		bool mapsToCharIndex () const      {return false;}
		const char* path () const;

	private:
		std::string _encname;
		std::vector<std::string> _table;
};

#endif
