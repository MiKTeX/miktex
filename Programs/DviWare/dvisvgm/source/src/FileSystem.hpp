/*************************************************************************
** FileSystem.hpp                                                       **
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

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>
#include <vector>

struct FileSystem
{
	static bool remove (const std::string &fname);
	static bool rename (const std::string &oldname, const std::string &newname);
	static bool copy (const std::string &src, const std::string &dest, bool remove_src=false);
	static uint64_t filesize (const std::string &fname);
	static std::string adaptPathSeperators (std::string path);
	static std::string getcwd ();
	static bool chdir (const std::string &dir);
	static bool exists (const std::string &fname);
	static bool mkdir (const std::string &dirname);
	static bool rmdir (const std::string &dirname);
	static int collect (const char *dirname, std::vector<std::string> &entries);
	static bool isDirectory (const std::string &fname);
	static bool isFile (const std::string &fname);
	static const char* userdir ();
	static const char* DEVNULL;
	static const char PATHSEP;
};

#endif
