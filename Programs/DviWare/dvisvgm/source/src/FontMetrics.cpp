/*************************************************************************
** FontMetrics.cpp                                                      **
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

#include <fstream>
#include <memory>
#include "FileFinder.hpp"
#include "FontMetrics.hpp"
#include "JFM.hpp"
#include "utility.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;


unique_ptr<FontMetrics> FontMetrics::read (const string &fontname) {
	const char *path = FileFinder::instance().lookup(fontname + ".tfm");
#if defined(MIKTEX_WINDOWS)
        ifstream ifs(EXPATH_(path), ios::binary);
#else
	ifstream ifs(path, ios::binary);
#endif
	if (!ifs)
		return unique_ptr<FontMetrics>();
	uint16_t id = 256*ifs.get();
	id += ifs.get();
	if (id == 9 || id == 11)  // Japanese font metric file?
		return util::make_unique<JFM>(ifs);
	return util::make_unique<TFM>(ifs);
}
