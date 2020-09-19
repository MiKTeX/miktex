/*************************************************************************
** InputBuffer.cpp                                                      **
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

#include <cmath>
#include <cstring>
#include "InputBuffer.hpp"

using namespace std;


StreamInputBuffer::StreamInputBuffer (istream &is, size_t bufsize)
	: _is(is), _bufsize(bufsize), _buf1(new uint8_t[_bufsize]), _buf2(new uint8_t[_bufsize]), _bufptr(_buf1)
{
	_size1 = fillBuffer(_buf1);
	_size2 = fillBuffer(_buf2);
}


StreamInputBuffer::~StreamInputBuffer () {
	delete [] _buf1;
	delete [] _buf2;
}


int StreamInputBuffer::get () {
	if (pos() == _size1) {
		if (_size2 == 0)
			return -1;
		swap(_buf1, _buf2);
		_size1 = _size2;
		_bufptr = _buf1;
		_size2 = fillBuffer(_buf2);
	}
	uint8_t c = *_bufptr++;
	return c;
}


/** Returns the next character to be read without skipping it.
 *  Same as peek(0). */
int StreamInputBuffer::peek () const {
	if (pos() < _size1)
		return *_bufptr;
	return _size2 > 0 ? *_buf2 : -1;
}


/** Returns the n-th next character without skipping it. */
int StreamInputBuffer::peek (size_t n) const {
	if (pos()+n < _size1)
		return *(_bufptr+n);
	if (pos()+n < _size1+_size2)
		return *(_buf2 + pos()+n-_size1);
	return -1;
}


/** Fills the buffer by reading a sequence of characters from the assigned
 *  input stream.
 *  @param[in] buf pointer to character buffer to be filled
 *  @return number of characters read */
int StreamInputBuffer::fillBuffer (uint8_t *buf) {
	if (_is && !_is.eof()) {
		_is.read((char*)buf, _bufsize);
		return  _is.gcount();
	}
	return 0;
}

///////////////////////////////////////////////

SplittedCharInputBuffer::SplittedCharInputBuffer (const char *buf1, size_t s1, const char *buf2, size_t s2) {
	_buf[0] = buf1;
	_buf[1] = buf2;
	_size[0] = buf1 ? s1 : 0;
	_size[1] = buf2 ? s2 : 0;
	_index = _size[0] ? 0 : 1;
}


int SplittedCharInputBuffer::get () {
	if (_size[_index] == 0)
		return -1;
	int ret = *_buf[_index]++;
	_size[_index]--;
	if (_index == 0 && _size[0] == 0)
		_index++;
	return ret;
}


int SplittedCharInputBuffer::peek () const {
	return _size[_index] ? *_buf[_index] : -1;
}


int SplittedCharInputBuffer::peek (size_t n) const {
	if (n < _size[_index])
		return _buf[_index][n];
	n -= _size[_index];
	if (_index == 0 && n < _size[1])
		return _buf[1][n];
	return -1;
}

///////////////////////////////////////////////


int TextStreamInputBuffer::get () {
	int c = StreamInputBuffer::get();
	if (c == '\n') {
		_line++;
		_col = 1;
	}
	else
		_col++;
	return c;
}


