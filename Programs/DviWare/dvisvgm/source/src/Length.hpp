/*************************************************************************
** Length.hpp                                                           **
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

#ifndef LENGTH_HPP
#define LENGTH_HPP

#include <string>
#include "MessageException.hpp"

#ifdef IN
#undef IN
#endif

struct UnitException : MessageException
{
	UnitException (const std::string &msg) : MessageException(msg) {}
};


class Length
{
	public:
		enum class Unit {PT, BP, CM, MM, IN, PC, DD, CC, SP};

	public:
		Length () : _pt(0) {}
		Length (double val, Unit unit=Unit::PT)         {set(val, unit);}
		Length (double val, const std::string &unitstr) {set(val, unitstr);}
		Length (const std::string &lenstr)              {set(lenstr);}
		void set (double val, Unit unit);
		void set (double val, std::string unit);
		void set (const std::string &lenstr);
		double pt () const {return _pt;}
		double in () const {return _pt*pt2in;}
		double bp () const {return _pt*pt2bp;}
		double cm () const {return _pt*pt2cm;}
		double mm () const {return _pt*pt2mm;}
		double pc () const {return _pt*pt2pc;}
		double dd () const {return _pt*pt2dd;}
		double cc () const {return _pt*pt2cc;}
		double sp () const {return _pt*pt2sp;}
		double get (Unit unit) const;
		std::string toString (Unit unit) const;

		static Unit stringToUnit (const std::string &unitstr);
		static std::string unitToString (Unit unit);

	public:
		static constexpr double pt2in = 1.0/72.27;
		static constexpr double pt2bp = pt2in*72;
		static constexpr double pt2cm = pt2in*2.54;
		static constexpr double pt2mm = pt2cm*10;
		static constexpr double pt2pc = 1.0/12;
		static constexpr double pt2dd = 1157.0/1238;
		static constexpr double pt2cc = pt2dd/12;
		static constexpr double pt2sp = 65536.0;

	private:
		double _pt;  // length in TeX point units (72.27pt = 1in)
};

#endif
