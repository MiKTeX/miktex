/*************************************************************************
** DLLoader.cpp                                                         **
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

#include <config.h>
#include "DLLoader.hpp"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

DLLoader::DLLoader (const char *dlname) : _handle(0)
{
	if (dlname && *dlname) {
#ifdef _WIN32
#if defined(MIKTEX)
          _handle = LoadLibraryW(UW_(dlname));
#else
		_handle = LoadLibrary(dlname);
#endif
#else
		_handle = dlopen(dlname, RTLD_LAZY);
#endif
	}
}


DLLoader::~DLLoader () {
	if (_handle) {
#ifdef _WIN32
		FreeLibrary(_handle);
#else
		dlclose(_handle);
#endif
	}
}


/** Loads a function or variable from the dynamic/shared library.
 *  @param[in] name name of function/variable to load
 *  @return pointer to loaded symbol, or 0 if the symbol could not be loaded */
void* DLLoader::loadSymbol (const char *name) {
	if (_handle) {
#ifdef _WIN32
		return (void*)GetProcAddress(_handle, name);
#else
		return dlsym(_handle, name);
#endif
	}
	return 0;
}
