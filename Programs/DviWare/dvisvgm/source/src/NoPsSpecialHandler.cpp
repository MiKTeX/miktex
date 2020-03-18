/*************************************************************************
** NoPsSpecialHandler.cpp                                               **
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

#include "Message.hpp"
#include "NoPsSpecialHandler.hpp"

using namespace std;


bool NoPsSpecialHandler::process (const string&, istream&, SpecialActions&) {
	_count++;
	return true;
}


void NoPsSpecialHandler::dviEndPage (unsigned pageno, SpecialActions &actions) {
	if (_count > 0) {
		string suffix = (_count > 1 ? "s" : "");
		Message::wstream(true) << _count << " PostScript special" << suffix << " ignored. The resulting SVG might look wrong.\n";
		_count = 0;
	}
}


vector<const char*> NoPsSpecialHandler::prefixes() const {
	vector<const char*> pfx {"header=", "psfile=", "PSfile=", "ps:", "ps::", "!", "\""};
	return pfx;
}
