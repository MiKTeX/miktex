/*************************************************************************
** Pair.hpp                                                             **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef PAIR_HPP
#define PAIR_HPP

#include <cmath>
#include <ostream>
#include "macros.hpp"

template <typename T>
class Pair
{
	public:
		Pair (T x=0, T y=0) : _x(x), _y(y) {}
		Pair operator += (const Pair &p)       {_x += p._x; _y += p._y; return *this;}
		Pair operator -= (const Pair &p)       {_x -= p._x; _y -= p._y; return *this;}
		Pair operator *= (T c)                 {_x *= c; _y *= c; return *this;}
		Pair operator /= (T c)                 {_x /= c; _y /= c; return *this;}
		Pair operator - () const               {return Pair(-_x, -_y);}
		Pair ortho () const                    {return Pair(-_y, _x);}
		double length () const                 {return std::sqrt(_x*_x + _y*_y);}
		bool operator == (const Pair &p) const {return _x == p._x && _y == p._y;}
		bool operator != (const Pair &p) const {return _x != p._x || _y != p._y;}
		T x () const                           {return _x;}
		T y () const                           {return _y;}
		void x (const T &xx)                   {_x = xx;}
		void y (const T &yy)                   {_y = yy;}
		std::ostream& write (std::ostream &os) const {return os << '(' << _x << ',' << _y << ')';}

	private:
		T _x, _y;
};

template <typename T>
inline Pair<T> abs (const Pair<T> &p) {
	return Pair<T>(std::abs(p.x()), std::abs(p.y()));
}

struct Pair32 : public Pair<int32_t>
{
	Pair32 (int32_t x=0, int32_t y=0) : Pair<int32_t>(x, y) {}
	explicit Pair32 (double x, double y) : Pair<int32_t>(int32_t(x+0.5), int32_t(y+0.5)) {}
	Pair32 (const Pair<int32_t> &p) : Pair<int32_t>(p) {}
};

typedef Pair<double> DPair;

template <typename T>
IMPLEMENT_ARITHMETIC_OPERATOR(Pair<T>, +)

template <typename T>
IMPLEMENT_ARITHMETIC_OPERATOR(Pair<T>, -)

template <typename T>
IMPLEMENT_ARITHMETIC_OPERATOR2(Pair<T>, T, *)

template <typename T>
IMPLEMENT_ARITHMETIC_OPERATOR2(Pair<T>, T, /)

template <typename T>
IMPLEMENT_OUTPUT_OPERATOR(Pair<T>)

IMPLEMENT_ARITHMETIC_OPERATOR2(Pair32, int32_t, *)
#endif
