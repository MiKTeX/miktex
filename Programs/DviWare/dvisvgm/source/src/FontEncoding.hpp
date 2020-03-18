/*************************************************************************
** FontEncoding.hpp                                                     **
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

#ifndef FONTENCODING_HPP
#define FONTENCODING_HPP

#include <string>
#include "Character.hpp"


struct CharMapID;
class PhysicalFont;

struct FontEncoding {
	virtual ~FontEncoding () =default;
	virtual Character decode (uint32_t c) const =0;
	virtual bool mapsToCharIndex () const =0;
	virtual const FontEncoding* findCompatibleBaseFontMap (const PhysicalFont *font, CharMapID &charmapID) const {return nullptr;}
	static FontEncoding* encoding (const std::string &encname);
};


struct NamedFontEncoding : public FontEncoding {
	virtual const char* name () const =0;
	virtual const char* path () const =0;
};


class FontEncodingPair : public FontEncoding {
	public:
		explicit FontEncodingPair (const FontEncoding *enc1) : _enc1(enc1), _enc2(nullptr) {}
		FontEncodingPair (const FontEncoding *enc1, const FontEncoding *enc2) : _enc1(enc1), _enc2(enc2) {}
		Character decode (uint32_t c) const override;
		bool mapsToCharIndex () const override;
		const FontEncoding* findCompatibleBaseFontMap (const PhysicalFont *font, CharMapID &charmapID) const override;
		const FontEncoding* enc1 () const       {return _enc1;}
		const FontEncoding* enc2 () const       {return _enc2;}
		void assign (const FontEncoding *enc);

	private:
		const FontEncoding *_enc1, *_enc2;
};

#endif
