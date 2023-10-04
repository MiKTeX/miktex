/*************************************************************************
** FontMetrics.cpp                                                      **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2023 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <fstream>
#include <memory>
#include "FileFinder.hpp"
#include "FontMetrics.hpp"
#include "JFM.hpp"
#include "OFM.hpp"
#include "utility.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;

static inline uint16_t read_uint16 (istream &is) {
	uint16_t val = 256*is.get();
	return val + is.get();
}


/** Reads the font metrics for a given font name from a TFM, JFM, or OFM file.
 *  @param[in] fontname name of font to get the metrics for.
 *  @return pointer to object that holds the font metrics, or nullptr if no matching file was found */
unique_ptr<FontMetrics> FontMetrics::read (const string &fontname) {
	unique_ptr<FontMetrics> metrics;
	const char *path = FileFinder::instance().lookup(fontname + ".ofm", false);
	if (!path)
		path = FileFinder::instance().lookup(fontname + ".tfm");
#if defined(MIKTEX_WINDOWS)
        ifstream ifs(EXPATH_(path), ios::binary);
#else
	ifstream ifs(path, ios::binary);
#endif
	if (ifs) {
		auto id = read_uint16(ifs);
		if (id == 0) {   // OFM?
			auto ofmLevel = read_uint16(ifs);
			if (ofmLevel == 0)
				metrics = util::make_unique<OFM0>();
			else if (ofmLevel == 1)
				metrics = util::make_unique<OFM1>();
			else
				throw FontMetricException("OFM level "+to_string(ofmLevel)+" not supported");
		}
		else if (id == 9 || id == 11)  // Japanese font metric file?
			metrics = util::make_unique<JFM>();
		else
			metrics = util::make_unique<TFM>();
		metrics->read(ifs);
	}
	return metrics;
}
