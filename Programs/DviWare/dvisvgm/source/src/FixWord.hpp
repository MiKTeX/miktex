/*************************************************************************
** FixWord.hpp                                                          **
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

#ifndef FIXWORD_HPP
#define FIXWORD_HPP

#include <cstdint>

class FixWord {
	public:
		FixWord () =default;
		FixWord (int32_t fw) : _value(fw) {}
		explicit operator double () const {return double(_value)/(1 << 20);}
		bool operator < (FixWord fw) const {return _value < fw._value;}
		bool operator == (FixWord fw) const {return _value == fw._value;}

	private:
		int32_t _value=0;
};

#endif
