/*************************************************************************
** FilePath.hpp                                                         **
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

#ifndef FILEPATH_HPP
#define FILEPATH_HPP

#include <string>
#include <utility>
#include <vector>

class FilePath {
	class Directory {
		public:
			Directory (const std::string &dir) : _dirstr(dir) {}
			Directory (std::string &&dir) : _dirstr(std::move(dir)) {}
			bool operator == (const Directory &dir) const;
			bool operator != (const Directory &dir) const {return !(*this == dir);}
			explicit operator std::string () const {return _dirstr;}

		private:
			std::string _dirstr;
	};

	public:
		FilePath (const std::string &path) {set(path);}
		FilePath (const std::string &path, bool isfile) : FilePath(path, isfile, "") {}
		FilePath (const std::string &path, bool isfile, const std::string &current_dir);
		void set (const std::string &path);
		std::string absolute (bool with_filename=true) const;
		std::string relative (std::string reldir="", bool with_filename=true) const;
		std::string basename () const;
		std::string suffix () const;
		void suffix (const std::string &s);
		size_t depth () const                    {return _dirs.size();}
		bool isFile () const                     {return !_fname.empty();}
		bool empty () const                      {return _dirs.empty() && _fname.empty();}
		const std::string& filename () const     {return _fname;}
		void filename (const std::string &fname) {_fname = fname;}

	protected:
		void init (std::string path, bool isfile, std::string current_dir);
		void add (const std::string &elem);

	private:
		std::vector<std::string> _dirs;
		std::string _fname;
#ifdef _WIN32
		char _drive;
#endif
};

#endif
