/*************************************************************************
** CMap.hpp                                                             **
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

#ifndef CMAP_HPP
#define CMAP_HPP

#include <algorithm>
#include <ostream>
#include <vector>
#include "FontEncoding.hpp"
#include "RangeMap.hpp"


struct CMap : public NamedFontEncoding {
	virtual bool vertical () const =0;
	virtual bool mapsToCID () const =0;
	virtual uint32_t cid (uint32_t c) const =0;
	virtual uint32_t bfcode (uint32_t cid) const =0;
	virtual std::string getROString () const =0;
	const char* path () const override;
	const FontEncoding* findCompatibleBaseFontMap (const PhysicalFont *font, CharMapID &charmapID) const override;
	bool mapsToCharIndex () const override {return mapsToCID();}

	Character decode (uint32_t c) const override {
		if (mapsToCID())
			return Character(Character::INDEX, cid(c));
		return Character(Character::CHRCODE, bfcode(c));
	}
};


struct IdentityCMap : public CMap {
	uint32_t cid (uint32_t c) const override      {return c;}
	uint32_t bfcode (uint32_t cid) const override {return 0;}
	std::string getROString () const override     {return "Adobe-Identity";}
	bool mapsToCID() const override               {return true;}
};


struct IdentityHCMap : public IdentityCMap {
	bool vertical () const override    {return false;}
	const char* name () const override {return "Identity-H";}
};


struct IdentityVCMap : public IdentityCMap {
	bool vertical () const override    {return true;}
	const char* name () const override {return "Identity-V";}
};


struct UnicodeCMap : public CMap {
	bool vertical () const override               {return false;}
	const char* name () const override            {return "unicode";}
	bool mapsToCID () const override              {return false;}
	const char* path () const override            {return nullptr;}
	uint32_t cid (uint32_t c) const override      {return c;}
	uint32_t bfcode (uint32_t cid) const override {return cid;}
	std::string getROString () const override     {return "";}
};


class SegmentedCMap : public CMap {
	friend class CMapReader;

	public:
		explicit SegmentedCMap (std::string fname) : _filename(std::move(fname)) {}
		const char* name () const override {return _filename.c_str();}
		uint32_t cid (uint32_t c) const override;
		uint32_t bfcode (uint32_t cid) const override;
		void addCIDRange (uint32_t first, uint32_t last, uint32_t cid)    {_cidranges.addRange(first, last, cid);}
		void addBFRange (uint32_t first, uint32_t last, uint32_t chrcode) {_bfranges.addRange(first, last, chrcode);}
		void write (std::ostream &os) const;
		bool vertical () const override  {return _vertical;}
		bool mapsToCID () const override {return _mapsToCID;}
		size_t numCIDRanges () const     {return _cidranges.numRanges();}
		size_t numBFRanges () const      {return _bfranges.numRanges();}
		std::string getROString () const override;

	private:
		std::string _filename;
		std::string _registry;
		std::string _ordering;
		CMap *_basemap = nullptr;
		bool _vertical = false;
		bool _mapsToCID = true;   // true: chrcode->CID, false: CID->charcode
		RangeMap _cidranges;
		RangeMap _bfranges;
};

#endif
