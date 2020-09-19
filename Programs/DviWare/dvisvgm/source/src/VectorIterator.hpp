/*************************************************************************
** VectorIterator.hpp                                                   **
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

#ifndef VECTORITERATOR_HPP
#define VECTORITERATOR_HPP

#include <vector>
#include "MessageException.hpp"


struct IteratorException : public MessageException {
	explicit IteratorException (const std::string &msg) : MessageException(msg) {}
};


template <typename T>
class VectorIterator {
	public:
		explicit VectorIterator (std::vector<T> &vec) : _vector(vec), _pos(0) {}

		VectorIterator operator ++ () {
			_pos++;
			return *this;
		}

		VectorIterator operator ++ (int) {
			VectorIterator it = *this;
			_pos++;
			return it;
		}

		VectorIterator operator -- () {
			_pos--;
			return *this;
		}

		VectorIterator operator -- (int) {
			VectorIterator it = *this;
			_pos--;
			return it;
		}

		VectorIterator operator += (int n) {
			_pos += n;
			return *this;
		}

		VectorIterator operator -= (int n) {
			_pos -= n;
			return *this;
		}

		VectorIterator operator + (int n) {
			return VectorIterator(_vector, _pos+n);
		}

		VectorIterator operator - (int n) {
			return VectorIterator(_vector, _pos-n);
		}

		T& operator * () {
			if (valid())
				return _vector[_pos];
			throw IteratorException("invalid access");
		}

		T* operator -> () {
			if (valid())
				return &_vector[_pos];
			throw IteratorException("invalid access");
		}

		bool operator == (const VectorIterator &it) const {return _pos == it._pos;}
		bool operator != (const VectorIterator &it) const {return _pos != it._pos;}
		bool operator <= (const VectorIterator &it) const {return _pos <= it._pos;}
		bool operator >= (const VectorIterator &it) const {return _pos >= it._pos;}
		bool operator < (const VectorIterator &it) const  {return _pos < it._pos;}
		bool operator > (const VectorIterator &it) const  {return _pos > it._pos;}
		bool valid () const {return _pos >= 0 && _pos < _vector.size();}
		void invalidate ()  {_pos = _vector.size();}
		void reset ()       {_pos = 0;}

	protected:
		VectorIterator (std::vector<T> &vec, size_t pos) : _vector(vec), _pos(pos) {}

	private:
		std::vector<T> &_vector;
		size_t _pos;
};

#endif
