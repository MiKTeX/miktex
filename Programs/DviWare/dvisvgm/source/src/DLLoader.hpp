/*************************************************************************
** DLLoader.hpp                                                         **
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

#ifndef DLLOADER_HPP
#define DLLOADER_HPP

#include <string>

#ifdef _WIN32
	#include "windows.hpp"
#else
	#include <dlfcn.h>
#endif


class DLLoader
{
	public:
		DLLoader () =delete;
		DLLoader (const std::string &dlname);
		virtual ~DLLoader () {closeLibrary();}
		bool loaded () const {return _handle != nullptr;}
		bool loadLibrary (const std::string &dlname);

	protected:
		void* loadSymbol (const char *name);
		void closeLibrary ();

	private:
#ifdef _WIN32
		HINSTANCE _handle;
#else
		void *_handle;
#endif
};

#endif
