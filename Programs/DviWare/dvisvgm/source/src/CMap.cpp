/*************************************************************************
** CMap.cpp                                                             **
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
#include <algorithm>
#include <sstream>
#include "CMap.h"
#include "CMapManager.h"
#include "FileFinder.h"

using namespace std;


const char* CMap::path () const {
	return FileFinder::lookup(name(), "cmap", false);
}


const FontEncoding* CMap::findCompatibleBaseFontMap (const PhysicalFont *font, CharMapID &charmapID) const {
	return CMapManager::instance().findCompatibleBaseFontMap(font, this, charmapID);
}

//////////////////////////////////////////////////////////////////////

/** Returns the RO (Registry-Ordering) string of the CMap. */
string SegmentedCMap::getROString() const {
	if (_registry.empty() || _ordering.empty())
		return "";
	return _registry + "-" + _ordering;
}


/** Returns the CID for a given character code. */
UInt32 SegmentedCMap::cid (UInt32 c) const {
	if (_cidranges.valueExists(c))
		return _cidranges.valueAt(c);
	if (_basemap)
		return _basemap->cid(c);
	return 0;
}


/** Returns the character code of a base font for a given CID. */
UInt32 SegmentedCMap::bfcode (UInt32 cid) const {
	if (_bfranges.valueExists(cid))
		return _bfranges.valueAt(cid);
	if (_basemap)
		return _basemap->bfcode(cid);
	return 0;
}


void SegmentedCMap::write (ostream &os) const {
	_cidranges.write(os);
}
