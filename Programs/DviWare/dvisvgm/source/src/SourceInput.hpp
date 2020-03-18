/*************************************************************************
** SourceInput.hpp                                                      **
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

#ifndef SOURCEINPUT_HPP
#define SOURCEINPUT_HPP

#include <fstream>
#include <string>


/** Helper class to handle temporary files. */
class TemporaryFile {
	public:
		~TemporaryFile () {close();}
		bool create ();
		bool opened () const {return _fd >= 0;}
		bool write (const char *buf, size_t len);
		bool close ();
		const std::string& path () const {return _path;}

	private:
		int _fd = -1;       ///< file descriptor assigned to the temporary file
		std::string _path;  ///< path to temporary file
};


class SourceInput {
	public:
		explicit SourceInput (const std::string &fname) : _fname(fname) {}
		std::istream& getInputStream (bool showMessages=false);
		std::string getFileName () const;
		std::string getMessageFileName () const;
		std::string getFilePath () const;

	private:
		const std::string &_fname; ///< name of file to read from
		TemporaryFile _tmpfile;    ///< temporary file used when reading from stdin
		std::ifstream _ifs;
};

#endif
