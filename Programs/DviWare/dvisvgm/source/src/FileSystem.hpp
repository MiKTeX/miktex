/*************************************************************************
** FileSystem.hpp                                                       **
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

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>
#include <vector>

class FileSystem {
	public:
		~FileSystem ();
		static bool remove (const std::string &fname);
		static bool rename (const std::string &oldname, const std::string &newname);
		static bool copy (const std::string &src, const std::string &dest, bool remove_src=false);
		static uint64_t filesize (const std::string &fname);
		static std::string ensureForwardSlashes (std::string path);
		static std::string getcwd ();
		static std::string tmpdir ();
		static bool chdir (const std::string &dir);
		static bool exists (const std::string &fname);
		static bool mkdir (const std::string &dirname);
		static bool rmdir (const std::string &dirname);
		static int collect (const std::string &dirname, std::vector<std::string> &entries);
		static bool isDirectory (const std::string &fname);
		static bool isFile (const std::string &fname);
		static const char* userdir ();
		static const char* DEVNULL;   ///< name of NULL device
		static const char PATHSEP;    ///< character used to separate folders/files in a path
		static std::string TMPDIR;    ///< set path to temporary folder

	protected:
		FileSystem () =default;
		bool system_tmpdir_available ();
		static const char* TMPSUBDIR; ///< subdirectory of the system's temporary folder
};

#endif
