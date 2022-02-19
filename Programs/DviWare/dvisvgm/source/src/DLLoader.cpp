/*************************************************************************
** DLLoader.cpp                                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2022 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include "DLLoader.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;


/** Creates a new DLLoader object and tries to load the given dynamic/shared library.
 *  @param[in] dlname name of library to load */
DLLoader::DLLoader (const string &dlname) : _handle(nullptr) {
	loadLibrary(dlname);
}


/** Releases the currently assigned dynamic/shared library and loads another one.
 *  @param[in] dlname name of library to load */
bool DLLoader::loadLibrary (const string &dlname) {
	closeLibrary();
	if (!dlname.empty()) {
#ifdef _WIN32
#if defined(MIKTEX)
                _handle = LoadLibraryW(EXPATH_(dlname).c_str());
#else
		_handle = LoadLibrary(dlname.c_str());
#endif
#else
		_handle = dlopen(dlname.c_str(), RTLD_LAZY);
#endif
	}
	return _handle != nullptr;
}


/** Releases the library currently assigned to the DLLoader object. */
void DLLoader::closeLibrary () {
	if (_handle) {
#ifdef _WIN32
		FreeLibrary(_handle);
#else
		dlclose(_handle);
#endif
		_handle = nullptr;
	}
}

