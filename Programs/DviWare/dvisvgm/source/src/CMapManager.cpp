/*************************************************************************
** CMapManager.cpp                                                      **
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
#include <sstream>
#include "CMap.h"
#include "CMapManager.h"
#include "CMapReader.h"
#include "FileFinder.h"
#include "Message.h"

using namespace std;


CMapManager::~CMapManager () {
	for (CMaps::iterator it=_cmaps.begin(); it != _cmaps.end(); ++it)
		delete it->second;
}


CMapManager& CMapManager::instance () {
	static CMapManager cmm;
	return cmm;
}


/** Loads a cmap and returns the corresponding object. */
CMap* CMapManager::lookup (const string &name) {
	CMaps::iterator it = _cmaps.find(name);
	if (it != _cmaps.end())
		return it->second;

	if (_includedCMaps.find(name) != _includedCMaps.end()) {
		_level = 0;
		ostringstream oss;
		oss << "circular reference of CMap " << name;
		throw CMapReaderException(oss.str());
	}

	CMap *cmap=0;
	if (name == "Identity-H")
		cmap = new IdentityHCMap;
	else if (name == "Identity-V")
		cmap = new IdentityVCMap;
	else if (name == "unicode")
		cmap = new UnicodeCMap;
	if (cmap) {
		_cmaps[name] = cmap;
		return cmap;
	}
	// Load cmap data of file <name> and also process all cmaps referenced by operator "usecmap".
	// This can lead to a sequence of further calls of lookup(). In order to prevent infinite loops
	// due to (disallowed) circular cmap inclusions, we keep track of all cmaps processed during
	// a sequence of inclusions.
	_includedCMaps.insert(name);  // save name of current cmap being processed
	_level++;                     // increase nesting level
	try {
		CMapReader reader;
		if (!(cmap = reader.read(name))) {
			_level = 1;
			Message::wstream(true) << "CMap file '" << name << "' not found\n";
		}
		_cmaps[name] = cmap;
	}
	catch (const CMapReaderException &e) {
		Message::estream(true) << "CMap file " << name << ": " << e.what() << "\n";
	}
	if (--_level == 0)            // back again at initial nesting level?
		_includedCMaps.clear();    // => names of included cmaps are no longer needed
	return cmap;
}


/** Looks for a base font CMap and a compatible encoding table in a given font. The CMap describe
 *  the mapping from CIDs to character codes where the latter are relative to the encoding table
 *  identified by charmapID.
 *  cmap:X->CID, bfmap:CID->Y, enctable:Y->CharCode
 *  @param[in] font look for available encoding tables in this font
 *  @param[in] cmap take the source registry-ordering pair from this CMap
 *  @param[out] charmapID ID of the compatible character map found in the given font
 *  @return base font CMap that maps from CIDs to character codes */
const CMap* CMapManager::findCompatibleBaseFontMap (const PhysicalFont *font, const CMap *cmap, CharMapID &charmapID) {
	if (!font || !cmap)
		return 0;

	static const struct CharMapIDToEncName {
		CharMapID id;
		const char *encname;
	} encodings[] = {
		{CharMapID::WIN_UCS4,         "UCS4"},
		{CharMapID::WIN_UCS2,         "UCS2"},
		{CharMapID::WIN_SHIFTJIS,     "90ms-RKSJ"},
		{CharMapID::WIN_PRC,          "GBK-EUC"},
		{CharMapID::WIN_BIG5,         "ETen-B5"},
		{CharMapID::WIN_WANSUNG,      "KSCms-UHC"},
		{CharMapID::MAC_JAPANESE,     "90pv-RKSJ"},
		{CharMapID::MAC_TRADCHINESE,  "B5pc"},
		{CharMapID::MAC_SIMPLCHINESE, "GBpc-EUC"},
		{CharMapID::MAC_KOREAN,       "KSCpc-EUC"}
	};

	// get IDs of all available charmaps in font
	vector<CharMapID> charmapIDs;
	font->collectCharMapIDs(charmapIDs);

	const bool is_unicode_map = bool(dynamic_cast<const UnicodeCMap*>(cmap));
	const size_t num_encodings = is_unicode_map ? 2 : sizeof(encodings)/sizeof(CharMapIDToEncName);

	// try to find a compatible encoding CMap
	const string ro = cmap->getROString();
	for (const CharMapIDToEncName *enc=encodings; enc < enc+num_encodings; enc++) {
		for (size_t i=0; i < charmapIDs.size(); i++) {
			if (enc->id == charmapIDs[i]) {
				string cmapname = ro+"-"+enc->encname;
				if (is_unicode_map || FileFinder::lookup(cmapname, "cmap", false)) {
					charmapID = enc->id;
					return is_unicode_map ? cmap : lookup(cmapname);
				}
			}
		}
	}
	return 0;
}

