/*************************************************************************
** MapLine.h                                                            **
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

#ifndef DVISVGM_MAPLINE_H
#define DVISVGM_MAPLINE_H

#include <istream>
#include <string>
#include "MessageException.h"


class InputReader;
class SubfontDefinition;


struct MapLineException : MessageException
{
	MapLineException (const std::string &msg) : MessageException(msg) {}
};


class MapLine
{
	public:
		MapLine (std::istream &is);
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
		void init ();
		bool isDVIPSFormat (const char *line) const;
		void parse (const char *line);
		void parseDVIPSLine (InputReader &ir);
		void parseDVIPDFMLine (InputReader &ir);
		void parseFilenameOptions (std::string opt);

	private:
		std::string _texname;     ///< TeX font name
		std::string _psname;      ///< PS font name
		std::string _fontfname;   ///< name of fontfile
		std::string _encname;     ///< name of encoding (without file suffix ".enc")
		SubfontDefinition *_sfd;  ///< subfont definition to be used
		int _fontindex;           ///< font index of file with multiple fonts (e.g. ttc files)
		double _slant, _bold, _extend;
};


#endif
