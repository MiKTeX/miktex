/*************************************************************************
** PdfSpecialHandler.cpp                                                **
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
#include <cstring>
#include "InputReader.h"
#include "MapLine.h"
#include "PdfSpecialHandler.h"
#include "FontMap.h"
#include "Message.h"

using namespace std;


PdfSpecialHandler::PdfSpecialHandler () : _maplineProcessed(false)
{
}


bool PdfSpecialHandler::process (const char *prefix, istream &is, SpecialActions &actions) {
	StreamInputReader ir(is);
	ir.skipSpace();
	string cmd = ir.getWord();
	ir.skipSpace();
	if (cmd == "mapline" || cmd == "mapfile") {
		// read mode selector ('+', '-', or '=')
		char modechar = '+';           // default mode (append if new, do not replace existing mapping)
		if (strchr("=+-", ir.peek()))  // leading modifier given?
			modechar = char(ir.get());
		else if (!_maplineProcessed) { // no modifier given?
			// remove default map entries if this is the first mapline/mapfile special called
			FontMap::instance().clear();
		}

		if (cmd == "mapline") {
			try {
				MapLine mapline(is);
				FontMap::instance().apply(mapline, modechar);
			}
			catch (const MapLineException &ex) {
				Message::wstream(true) << "pdf:mapline: " << ex.what() << '\n';
			}
		}
		else { // mapfile
			string fname = ir.getString();
			if (!FontMap::instance().read(fname, modechar))
				Message::wstream(true) << "can't open map file " << fname << '\n';
		}
		_maplineProcessed = true;
	}
	return true;
}


const char** PdfSpecialHandler::prefixes () const {
	static const char *pfx[] = {"pdf:", 0};
	return pfx;
}
