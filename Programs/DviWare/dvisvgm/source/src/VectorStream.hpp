/*************************************************************************
** VectorStream.hpp                                                     **
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

#ifndef VECTORSTREAM_HPP
#define VECTORSTREAM_HPP

#include <istream>
#include <vector>

template <typename T>
class VectorStreamBuffer : public std::streambuf {
	public:
		explicit VectorStreamBuffer (const std::vector<T> &v) {
			if (!v.empty()) {
				_begin = _curr = &v[0];
				_end = &v[0]+v.size();
			}
		}

	protected:
		int_type underflow () override {
			return _curr == _end ? traits_type::eof() : traits_type::to_int_type(*_curr);
		}

		int_type uflow() override {
			return _curr == _end ? traits_type::eof() : traits_type::to_int_type(*_curr++);
		}

		std::streamsize showmanyc () override {return _end-_curr;}

		int_type pbackfail (int_type c) override {
			if (_curr == _begin || (c != traits_type::eof() && c != _curr[-1]))
				return traits_type::eof();
			return traits_type::to_int_type(*--_curr);
		}

		pos_type seekoff (off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which=std::ios_base::in) override {
			switch (dir) {
				case std::ios_base::cur:
					_curr += off; break;
				case std::ios_base::beg:
					_curr = _begin+off; break;
				case std::ios_base::end:
					_curr = _end-off; break;
				default:
					break;
			}
			return _curr-_begin;
		}

	private:
		const T *_begin=nullptr;
		const T *_end=nullptr;
		const T *_curr=nullptr;
};


template <typename T>
class VectorInputStream : public std::istream {
	public:
		explicit VectorInputStream (const std::vector<T> &source) : std::istream(&_buf), _buf(source) {}

	private:
		VectorStreamBuffer<T> _buf;
};

#endif
