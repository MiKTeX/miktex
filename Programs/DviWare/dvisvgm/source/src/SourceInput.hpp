/*************************************************************************
** SourceInput.hpp                                                      **
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

#ifndef DVIINPUT_HPP
#define DVIINPUT_HPP

#include <fstream>
#include <string>

class SourceInput {
	public:
		SourceInput (const std::string &fname) : _fname(fname) {}
		~SourceInput ();
		std::istream& getInputStream (bool showMessages=false);
		std::string getFileName () const;
		std::string getMessageFileName () const;
		std::string getFilePath () const;

	private:
		const std::string &_fname; ///< name of file to read from
		std::string _tmpfilepath;  ///< path of temporary file used when reading from stdin
		std::ifstream _ifs;
};

#endif