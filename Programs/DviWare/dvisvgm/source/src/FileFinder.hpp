/*************************************************************************
** FileFinder.hpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef FILEFINDER_HPP
#define FILEFINDER_HPP

#include <memory>
#include <set>
#include <string>

class MiKTeXCom;

class FileFinder
{
	public:
		static void init (const std::string &argv0, const std::string &progname, bool enable_mktexmf);
		static FileFinder& instance ();
		std::string version () const;
		void addLookupDir (const std::string &path);
		const char* lookup (const std::string &fname, const char *ftype, bool extended=true) const;
		const char* lookup (const std::string &fname, bool extended=true) const {return lookup(fname, 0, extended);}

	protected:
		FileFinder ();
		const char* findFile (const std::string &fname, const char *ftype) const;
		const char* findMappedFile (std::string fname) const;
		const char* mktex (const std::string &fname) const;

	private:
		static std::string _argv0;
		static std::string _progname;
		static bool _enableMktex;
		std::set<std::string> _additionalDirs;
#ifdef MIKTEX_COM
		std::unique_ptr<MiKTeXCom> _miktex;
#endif
};

#endif
