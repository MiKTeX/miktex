/*************************************************************************
** PreScanDVIReader.cpp                                                 **
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

#include "DVIActions.hpp"
#include "PreScanDVIReader.hpp"

using namespace std;


PreScanDVIReader::PreScanDVIReader (std::istream &is, DVIActions *actions)
	: BasicDVIReader(is), _actions(actions)
{
}


void PreScanDVIReader::cmdBop (int) {
	_currentPageNumber++;
	BasicDVIReader::cmdBop(0);
}


void PreScanDVIReader::cmdXXX (int len) {
	uint32_t numBytes = readUnsigned(len);
	string s = readString(numBytes);
	if (_actions)
		_actions->special(s, 0, true);  // pre-process special
}
