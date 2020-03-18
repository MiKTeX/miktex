/*************************************************************************
** Length.hpp                                                           **
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

#ifndef LENGTH_HPP
#define LENGTH_HPP

#include <map>
#include <vector>
#include "MessageException.hpp"

#ifdef IN
#undef IN
#endif

struct UnitException : MessageException {
	explicit UnitException (const std::string &msg) : MessageException(msg) {}
};


class Length {
	public:
		enum class Unit {PT, BP, CM, MM, IN, PC, DD, CC, SP};

	public:
		constexpr Length () =default;
		explicit constexpr Length (long double pt) : _pt(static_cast<double>(pt)) {}
		Length (double val, Unit unit)                  {set(val, unit);}
		Length (double val, const std::string &unitstr) {set(val, unitstr);}
		explicit Length (const std::string &lenstr)     {set(lenstr);}
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
		bool operator == (Length len) const {return _pt == len._pt;}
		bool operator != (Length len) const {return _pt != len._pt;}

		static Unit stringToUnit (const std::string &unitstr);
		static std::string unitToString (Unit unit);
		static std::map<std::string,Unit> getUnits ();

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
		double _pt=0;  // length in TeX point units (72.27pt = 1in)
};


constexpr Length operator "" _pt (long double pt) {return Length(pt);}
constexpr Length operator "" _bp (long double bp) {return Length(bp/Length::pt2bp);}
constexpr Length operator "" _mm (long double mm) {return Length(mm/Length::pt2mm);}
constexpr Length operator "" _in (long double in) {return Length(in/Length::pt2in);}
constexpr Length operator "" _pt (unsigned long long pt) {return Length(static_cast<double>(pt));}
constexpr Length operator "" _bp (unsigned long long bp) {return Length(static_cast<double>(bp)/Length::pt2bp);}
constexpr Length operator "" _mm (unsigned long long mm) {return Length(static_cast<double>(mm)/Length::pt2mm);}
constexpr Length operator "" _in (unsigned long long in) {return Length(static_cast<double>(in)/Length::pt2in);}

#endif
