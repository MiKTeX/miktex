/*************************************************************************
** CMapManager.hpp                                                      **
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

#ifndef CMAPMANAGER_HPP
#define CMAPMANAGER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "CharMapID.hpp"

struct CMap;
struct FontEncoding;
class FontEncodingPair;
class PhysicalFont;

class CMapManager
{
	public:
		CMap* lookup (const std::string &name);
		const CMap* findCompatibleBaseFontMap (const PhysicalFont *font, const CMap *cmap, CharMapID &charmapID);
		static CMapManager& instance ();

	protected:
		CMapManager () : _level(0) {}

	private:
		std::unordered_map<std::string, std::unique_ptr<CMap>> _cmaps;  ///< loaded cmaps
		std::unordered_set<std::string> _includedCMaps;  ///< names of cmaps loaded by "usecmap"
		int _level;    ///< current inclusion depth; >0 if a cmap loaded by "usecmap" is being processed
};

#endif
