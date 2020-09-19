/*************************************************************************
** GFReader.hpp                                                         **
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

#ifndef GFREADER_HPP
#define GFREADER_HPP

#include <istream>
#include <string>
#include <unordered_map>
#include "Bitmap.hpp"
#include "MessageException.hpp"


class CharInfo;


struct GFException : public MessageException {
	explicit GFException (const std::string &msg) : MessageException(msg) {}
};


class GFReader {
	struct CharInfo {
		CharInfo () : dx(0), dy(0), width(0), location(0) {}
		CharInfo (int32_t dxx, int32_t dyy, int32_t w, uint32_t p) : dx(dxx), dy(dyy), width(w), location(p) {}

		int32_t dx, dy;
		int32_t width;  // 2^24 * (true width)/(design size)
		uint32_t location;
	};

	public:
		explicit GFReader (std::istream &is) : _in(is) {}
		virtual ~GFReader () =default;
		virtual bool executeChar (uint8_t c);
		virtual bool executeAllChars ();
		bool executePreamble ();
		bool executePostamble ();
		virtual void preamble (const std::string &str) {}
		virtual void postamble () {}
		virtual void beginChar (uint32_t c) {}
		virtual void endChar (uint32_t c) {}
		virtual void special (const std::string &str) {}
		virtual void numspecial (int32_t y) {}
		const Bitmap& getBitmap () const   {return _bitmap;}
		double getDesignSize () const      {return _designSize;}
		double getHPixelsPerPoint () const {return _hppp;}
		double getVPixelsPerPoint () const {return _vppp;};
		double getCharWidth (uint32_t c) const;
		uint32_t getChecksum () const      {return _checksum;}

	protected:
		int32_t readSigned (int bytes);
		uint32_t readUnsigned (int bytes);
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
		int32_t _minX=0, _maxX=0, _minY=0, _maxY=0;
		int32_t _x=0, _y=0;          ///< current pen location (pixel units)
		int32_t _currentChar=0;
		Bitmap _bitmap;              ///< bitmap of current char
		double _designSize=0;        ///< designSize in PS points
		double _hppp=0, _vppp=0;     ///< horizontal and vertical pixel per point
		uint32_t _checksum=0;
		std::unordered_map<uint8_t,CharInfo> _charInfoMap;
		bool _insideCharDef=false;   ///< true if inside a character definition (between BOC and EOC)
		bool _penDown=false;
};

#endif
