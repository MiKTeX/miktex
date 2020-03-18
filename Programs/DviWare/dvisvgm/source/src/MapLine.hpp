/*************************************************************************
** MapLine.hpp                                                          **
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

#ifndef MAPLINE_HPP
#define MAPLINE_HPP

#include <istream>
#include <sstream>
#include <string>
#include "MessageException.hpp"


class InputReader;
class SubfontDefinition;


struct MapLineException : MessageException {
	explicit MapLineException (const std::string &msg) : MessageException(msg) {}
};


class MapLine {
	public:
		explicit MapLine (std::istream &is);
		explicit MapLine (std::string str);
		const std::string& texname () const   {return _texname;}
		const std::string& psname () const    {return _psname;}
		const std::string& fontfname () const {return _fontfname;}
		const std::string& encname () const   {return _encname;}
		int fontindex () const                {return _fontindex;}
		double bold () const                  {return _bold;}
		double slant () const                 {return _slant;}
		double extend () const                {return _extend;}
		SubfontDefinition* sfd () const       {return _sfd;}

	protected:
		MapLine () =default;
		bool isDVIPSFormat (const char *line) const;
		void parse (const char *line);
		void parseDVIPSLine (InputReader &ir);
		void parseDVIPDFMLine (InputReader &ir);
		void parseFilenameOptions (std::string opt);

	private:
		std::string _texname;             ///< TeX font name
		std::string _psname;              ///< PS font name
		std::string _fontfname;           ///< name of fontfile
		std::string _encname;             ///< name of encoding (without file suffix ".enc")
		SubfontDefinition *_sfd=nullptr;  ///< subfont definition to be used
		int _fontindex=0;                 ///< font index of file with multiple fonts (e.g. ttc files)
		double _slant=0, _bold=0, _extend=1;
};


#endif
