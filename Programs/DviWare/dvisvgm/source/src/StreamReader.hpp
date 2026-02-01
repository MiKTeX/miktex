/*************************************************************************
** StreamReader.hpp                                                     **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef STREAMREADER_HPP
#define STREAMREADER_HPP

#include <istream>
#include <string>
#include <vector>
#include "MessageException.hpp"

class HashFunction;

class StreamReader {
	public:
		explicit StreamReader (std::istream &is) : _is(&is) {}
		virtual ~StreamReader () =default;
		bool isStreamValid () const {return _is;}
		bool eof () const           {return _is->eof();}
		void clearStream () const   {_is->clear();}
		std::istream& replaceStream (std::istream &s);
		uint32_t readUnsigned (int n) const;
		uint32_t readUnsigned (int n, HashFunction &hashfunc) const;
		int32_t readSigned (int n) const;
		int32_t readSigned (int n, HashFunction &hashfunc) const;
		std::string readString () const;
		std::string readString (HashFunction &hashfunc, bool finalZero=false) const;
		std::string readString (int length) const;
		std::string readString (int length, HashFunction &hashfunc) const;
		std::vector<uint8_t> readBytes (int n) const;
		std::vector<uint8_t> readBytes (int n, HashFunction &hash) const;
		std::vector<char> readBytesAsChars (int n) const;
		int readByte () const           {return _is->get();}
		int readByte (HashFunction &hashfunc) const;
		void seek (std::streampos pos, std::ios::seekdir dir) const {_is->seekg(pos, dir);}
		void seek (std::streampos pos) const {_is->seekg(pos);}
		void skip (std::streampos n) const   {_is->seekg(n, std::ios::cur);}
		std::streampos tell () const {return _is->tellg();}
		int peek () const            {return _is->peek();}
		std::streampos tellg () const {return _is->tellg();}

	protected:
		std::istream& getInputStream () const {return *_is;}

	private:
		std::istream *_is;
};


struct StreamReaderException : MessageException {
	explicit StreamReaderException (const std::string &msg) : MessageException(msg) {}
};

#endif
