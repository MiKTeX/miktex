/*************************************************************************
** EncFile.hpp                                                          **
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

#ifndef ENCFILE_HPP
#define ENCFILE_HPP

#include <istream>
#include <map>
#include <string>
#include <vector>
#include "FontEncoding.hpp"


class EncFile : public NamedFontEncoding {
	public:
		explicit EncFile (std::string encname);
		void read ();
		void read (std::istream &is);
		int size () const {return _table.size();}
		const char* name () const override {return _encname.c_str();}
		const char* charName (uint32_t c) const;
		Character decode (uint32_t c) const override {return Character(charName(c));}
		bool mapsToCharIndex () const override {return false;}
		const char* path () const override;

	private:
		std::string _encname;
		std::vector<std::string> _table;
};

#endif
