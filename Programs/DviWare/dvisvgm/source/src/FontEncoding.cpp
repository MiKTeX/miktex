/*************************************************************************
** FontEncoding.cpp                                                     **
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

#include "CMap.hpp"
#include "CMapManager.hpp"
#include "EncFile.hpp"
#include "FileFinder.hpp"
#include "FontEncoding.hpp"
#include "utility.hpp"

using namespace std;

/** Returns the encoding object for a given encoding name.
 * @param[in] encname name of the encoding to lookup
 * @return pointer to encoding object, or 0 if there is no encoding defined */
FontEncoding* FontEncoding::encoding (const string &encname) {
	if (encname.empty())
		return nullptr;

	using EncodingMap = unordered_map<string, unique_ptr<EncFile>>;
	static EncodingMap encmap;
	// initially, try to find an .enc file with the given name
	auto it = encmap.find(encname);
	if (it != encmap.end())
		return it->second.get();
	if (FileFinder::instance().lookup(encname + ".enc", false)) {
		auto state = encmap.emplace(encname, util::make_unique<EncFile>(encname));
		return state.first->second.get();
	}
	// no .enc file found => try to find a CMap
	if (CMap *cmap = CMapManager::instance().lookup(encname))
		return cmap;
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////

Character FontEncodingPair::decode (uint32_t c) const {
	if (_enc1) {
		Character chr = _enc1->decode(c);
		if (_enc2 && chr.type() != Character::NAME)
			chr = _enc2->decode(chr.number());
		return chr;
	}
	return Character(Character::INDEX, 0);
}


bool FontEncodingPair::mapsToCharIndex () const {
	if (_enc2)
		return _enc2->mapsToCharIndex();
	if (_enc1)
		return _enc1->mapsToCharIndex();
	return false;
}


const FontEncoding* FontEncodingPair::findCompatibleBaseFontMap (const PhysicalFont *font, CharMapID &charmapID) const {
	if (_enc2)
		return _enc2->findCompatibleBaseFontMap(font, charmapID);
	if (_enc1)
		return _enc1->findCompatibleBaseFontMap(font, charmapID);
	return nullptr;
}


void FontEncodingPair::assign (const FontEncoding *enc) {
	if (!_enc1)
		_enc1 = enc;
	else
		_enc2 = enc;
}
