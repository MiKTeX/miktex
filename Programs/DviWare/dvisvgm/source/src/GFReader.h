/*************************************************************************
** GFReader.h                                                           **
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

#ifndef DVISVGM_GFREADER_H
#define DVISVGM_GFREADER_H

#include <istream>
#include <map>
#include <string>
#include "Bitmap.h"
#include "MessageException.h"
#include "types.h"


class CharInfo;


struct GFException : public MessageException
{
	GFException (const std::string &msg) : MessageException(msg) {}
};


class GFReader
{
	struct CharInfo
	{
		CharInfo () : dx(0), dy(0), width(0), location(0) {}
		CharInfo (Int32 dxx, Int32 dyy, Int32 w, UInt32 p) : dx(dxx), dy(dyy), width(w), location(p) {}

		Int32 dx, dy;
		Int32 width;  // 2^24 * (true width)/(design size)
		UInt32 location;
	};

	typedef std::map<UInt8,CharInfo>::iterator Iterator;
	typedef std::map<UInt8,CharInfo>::const_iterator ConstIterator;
	public:
		GFReader (std::istream &is);
		virtual ~GFReader () {}
		bool executeChar (UInt8 c);
		bool executeAllChars ();
		bool executePreamble ();
		bool executePostamble ();
		virtual void preamble (const std::string &str) {}
		virtual void postamble () {}
		virtual void beginChar (UInt32 c) {}
		virtual void endChar (UInt32 c) {}
		virtual void special (std::string str) {}
		virtual void numspecial (Int32 y) {}
		const Bitmap& getBitmap () const {return _bitmap;}
		double getDesignSize () const;
		double getHPixelsPerPoint () const;
		double getVPixelsPerPoint () const;
		double getCharWidth (UInt32 c) const;
		UInt32 getChecksum () const  {return _checksum;}

	protected:
		Int32 readSigned (int bytes);
		UInt32 readUnsigned (int bytes);
		std::string readString (int len);
		int executeCommand ();
		std::istream& getInputStream () const {return _in;}

		void cmdPre (int);
		void cmdPost (int);
		void cmdPostPost (int);
		void cmdPaint0 (int pixels);
		void cmdPaint (int len);
		void cmdBoc (int);
		void cmdBoc1 (int);
		void cmdEoc (int);
		void cmdSkip (int len);
		void cmdNewRow (int col);
		void cmdXXX (int len);
		void cmdYYY (int);
		void cmdNop (int);
		void cmdCharLoc0 (int);
		void cmdCharLoc (int);

	private:
		std::istream &_in;
		Int32 _minX, _maxX, _minY, _maxY;
		Int32 _x, _y;            ///< current pen location (pixel units)
		Int32 _currentChar;
		Bitmap _bitmap;          ///< bitmap of current char
		FixWord _designSize;     ///< designSize
		ScaledInt _hppp, _vppp;  ///< horizontal and vertical pixel per point
		UInt32 _checksum;
		std::map<UInt8,CharInfo> _charInfoMap;
		bool _insideCharDef;     ///< true if inside a character definition (between BOC and EOC)
		bool _penDown;
};

#endif
