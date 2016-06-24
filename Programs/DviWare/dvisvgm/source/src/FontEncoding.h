/*************************************************************************
** FontEncoding.h                                                       **
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

#ifndef DVISVGM_FONTENCODING_H
#define DVISVGM_FONTENCODING_H

#include <string>
#include "Character.h"
#include "types.h"


struct CharMapID;
class PhysicalFont;

struct FontEncoding
{
	virtual ~FontEncoding () {}
	virtual Character decode (UInt32 c) const =0;
	virtual bool mapsToCharIndex () const =0;
	virtual const FontEncoding* findCompatibleBaseFontMap (const PhysicalFont *font, CharMapID &charmapID) const {return 0;}
	static FontEncoding* encoding (const std::string &encname);
};


struct NamedFontEncoding : public FontEncoding
{
	virtual const char* name () const =0;
	virtual const char* path () const =0;
};


class FontEncodingPair : public FontEncoding
{
	public:
		FontEncodingPair (const FontEncoding *enc1) : _enc1(enc1), _enc2(0) {}
		FontEncodingPair (const FontEncoding *enc1, const FontEncoding *enc2) : _enc1(enc1), _enc2(enc2) {}
		Character decode (UInt32 c) const;
		bool mapsToCharIndex () const;
		const FontEncoding* findCompatibleBaseFontMap (const PhysicalFont *font, CharMapID &charmapID) const;
		const FontEncoding* enc1 () const       {return _enc1;}
		const FontEncoding* enc2 () const       {return _enc2;}
		void assign (const FontEncoding *enc);

	private:
		const FontEncoding *_enc1, *_enc2;
};

#endif
