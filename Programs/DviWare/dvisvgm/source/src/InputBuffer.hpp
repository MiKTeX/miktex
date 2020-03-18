/*************************************************************************
** InputBuffer.hpp                                                      **
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

#ifndef INPUTBUFFER_HPP
#define INPUTBUFFER_HPP

#include <algorithm>
#include <cstring>
#include <istream>
#include <string>
#include <ostream>

struct InputBuffer {
	virtual ~InputBuffer () =default;
	virtual int get () =0;
	virtual int peek () const =0;
	virtual int peek (size_t n) const =0;
	virtual bool eof () const =0;
	virtual void invalidate () =0;
};


class StreamInputBuffer : public InputBuffer {
	public:
		explicit StreamInputBuffer (std::istream &is, size_t bufsize=1024);
		~StreamInputBuffer () override;
		int get () override;
		int peek () const override;
		int peek (size_t n) const override;
		bool eof () const override {return pos() == _size1 && _size2 == 0;}
		void invalidate () override {_bufptr = _buf1+_size1; _size2 = 0;}

	protected:
		int fillBuffer (uint8_t *buf);
		size_t pos () const  {return _bufptr-_buf1;}

	private:
		std::istream &_is;
		const size_t _bufsize;  ///< maximal number of bytes each buffer can hold
		uint8_t *_buf1;         ///< pointer to first buffer
		uint8_t *_buf2;         ///< pointer to second buffer
		size_t _size1;          ///< number of bytes in buffer 1
		size_t _size2;          ///< number of bytes in buffer 2
		uint8_t *_bufptr;       ///< pointer to next byte to read
};


class StringInputBuffer : public InputBuffer {
	public:
		explicit StringInputBuffer (const std::string &str) : _str(&str) {}
		void assign (const std::string &str) {_str = &str; _pos=0;}
		int get () override                  {return _pos < _str->length() ? _str->at(_pos++) : -1;}
		int peek () const override           {return _pos < _str->length() ? _str->at(_pos) : -1;}
		int peek (size_t n) const override   {return _pos+n < _str->length() ? _str->at(_pos+n) : -1;}
		bool eof () const override           {return _pos >= _str->length();}
		void invalidate () override          {_pos = _str->length();}

	private:
		const std::string *_str;
		size_t _pos=0;
};


class CharInputBuffer : public InputBuffer {
	public:
		CharInputBuffer (const char *buf, size_t size) : _pos(buf), _size(buf ? size : 0) {}

		int get () override {
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

		void assign (const char *buf)      {assign(buf, std::strlen(buf));}
		int peek () const override         {return _size > 0 ? *_pos : -1;}
		int peek (size_t n) const override {return _size >= n ? _pos[n] : -1;}
		bool eof () const override         {return _size == 0;}
		void invalidate () override        {_size = 0;}

	private:
		const char *_pos;
		size_t _size;
};


class SplittedCharInputBuffer : public InputBuffer {
	public:
		SplittedCharInputBuffer (const char *buf1, size_t s1, const char *buf2, size_t s2);
		int get () override;
		int peek () const override;
		int peek (size_t n) const override;
		bool eof () const override  {return _size[_index] == 0;}
		void invalidate () override {_size[_index] = 0;}

	private:
		const char *_buf[2];
		size_t _size[2];
		int _index;
};


class TextStreamInputBuffer : public StreamInputBuffer {
	public:
		explicit TextStreamInputBuffer (std::istream &is) : StreamInputBuffer(is) {}
		int get () override;
		int line () const {return _line;}
		int col () const {return _col;}

	private:
		int _line=1, _col=1;
};

#endif
