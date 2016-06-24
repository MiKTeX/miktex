/*************************************************************************
** InputBuffer.h                                                        **
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

#ifndef DVISVGM_INPUTBUFFER_H
#define DVISVGM_INPUTBUFFER_H

#include <algorithm>
#include <cstring>
#include <istream>
#include <string>
#include <ostream>
#include "types.h"

struct InputBuffer
{
	virtual ~InputBuffer () {}
	virtual int get () =0;
	virtual int peek () const =0;
	virtual int peek (size_t n) const =0;
	virtual bool eof () const =0;
	virtual void invalidate () =0;
};


class StreamInputBuffer : public InputBuffer
{
	public:
		StreamInputBuffer (std::istream &is, size_t bufsize=1024);
		~StreamInputBuffer ();
		int get ();
		int peek () const;
		int peek (size_t n) const;
		bool eof () const  {return pos() == _size1 && _size2 == 0;}
		void invalidate () {_bufptr = _buf1+_size1; _size2 = 0;}

	protected:
		int fillBuffer (UInt8 *buf);
		size_t pos () const  {return _bufptr-_buf1;}

	private:
		std::istream &_is;
		const size_t _bufsize;  ///< maximal number of bytes each buffer can hold
		UInt8 *_buf1;        ///< pointer to first buffer
		UInt8 *_buf2;        ///< pointer to second buffer
		size_t _size1;     ///< number of bytes in buffer 1
		size_t _size2;     ///< number of bytes in buffer 2
		UInt8 *_bufptr;      ///< pointer to next byte to read
};


class StringInputBuffer : public InputBuffer
{
	public:
		StringInputBuffer (const std::string &str) : _str(str), _pos(0) {}
		int get ()                  {return _pos < _str.length() ? _str[_pos++] : -1;}
		int peek () const           {return _pos < _str.length() ? _str[_pos] : -1;}
		int peek (size_t n) const {return _pos+n < _str.length() ? _str[_pos+n] : -1;}
		bool eof () const           {return _pos >= _str.length();}
		void invalidate ()          {_pos = _str.length();}

	private:
		const std::string &_str;
		size_t _pos;
};


class CharInputBuffer : public InputBuffer
{
	public:
		CharInputBuffer (const char *buf, size_t size) : _pos(buf), _size(buf ? size : 0) {}

		int get () {
			if (_size == 0)
				return -1;
			else {
				_size--;
				return *_pos++;
			}
		}


		void assign (const char *buf, size_t size) {
			_pos = buf;
			_size = size;
		}

		void assign (const char *buf) {assign(buf, std::strlen(buf));}
		int peek () const             {return _size > 0 ? *_pos : -1;}
		int peek (size_t n) const     {return _size >= n ? _pos[n] : -1;}
		bool eof () const             {return _size == 0;}
		void invalidate ()            {_size = 0;}

	private:
		const char *_pos;
		size_t _size;
};


class SplittedCharInputBuffer : public InputBuffer
{
	public:
		SplittedCharInputBuffer (const char *buf1, size_t s1, const char *buf2, size_t s2);
		int get ();
		int peek () const;
		int peek (size_t n) const;
		bool eof () const   {return _size[_index] == 0;}
		void invalidate ()  {_size[_index] = 0;}

	private:
		const char *_buf[2];
		size_t _size[2];
		int _index;
};


class TextStreamInputBuffer : public StreamInputBuffer
{
	public:
		TextStreamInputBuffer (std::istream &is) : StreamInputBuffer(is), _line(1), _col(1) {}
		int get ();
		int line () const {return _line;}
		int col () const {return _col;}

	private:
		int _line, _col;
};

#endif
