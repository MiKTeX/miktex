/*************************************************************************
** Length.cpp                                                           **
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

#include <sstream>
#include "InputReader.h"
#include "Length.h"

using namespace std;

const double Length::pt2in = 1.0/72.27;
const double Length::pt2bp = pt2in*72;
const double Length::pt2cm = pt2in*2.54;
const double Length::pt2mm = pt2cm*10;
const double Length::pt2pc = 1.0/12;
const double Length::pt2dd = 1157.0/1238;
const double Length::pt2cc = pt2dd/12;
const double Length::pt2sp = 65536.0;


void Length::set (const string &lenstr) {
	switch (lenstr.length()) {
		case 0:
			_pt = 0;
			break;
		case 1:
			if (isdigit(lenstr[0]))
				_pt = lenstr[0] - '0';
			else
				throw UnitException(string("invalid length: ")+lenstr);
			break;
		default:
			istringstream iss(lenstr);
			StreamInputReader ir(iss);
			double val;
			if (!ir.parseDouble(val))
				throw UnitException(string("invalid length: ")+lenstr);
			string unit = ir.getWord();
			set(val, unit);
	}
}


void Length::set (double val, Unit unit) {
	switch (unit) {
		case PT: _pt = val; break;
		case BP: _pt = val/pt2bp; break;
		case IN: _pt = val/pt2in; break;
		case CM: _pt = val/pt2cm; break;
		case MM: _pt = val/pt2mm; break;
		case PC: _pt = val/pt2pc; break;
		case DD: _pt = val/pt2dd; break;
		case CC: _pt = val/pt2cc; break;
		case SP: _pt = val/pt2sp; break;
		default:
			// this isn't supposed to happen
			ostringstream oss;
			oss << "invalid length unit: (" << unit << ")";
			throw UnitException(oss.str());
	}
}


double Length::get (Unit unit) const {
	switch (unit) {
		case PT: return pt();
		case BP: return bp();
		case IN: return in();
		case CM: return cm();
		case MM: return mm();
		case PC: return pc();
		case DD: return dd();
		case CC: return cc();
		case SP: return sp();
	}
	// this isn't supposed to happen
	ostringstream oss;
	oss << "invalid length unit: (" << unit << ")";
	throw UnitException(oss.str());
}


string Length::toString (Unit unit) const {
	ostringstream oss;
	oss << get(unit) << unitToString(unit);
	return oss.str();
}


#define UNIT(c1, c2) ((c1 << 8)|c2)

Length::Unit Length::stringToUnit (const std::string &unitstr) {
	if (unitstr.length() == 2) {
		switch (UNIT(unitstr[0], unitstr[1])) {
			case UNIT('p','t'): return PT;
			case UNIT('b','p'): return BP;
			case UNIT('i','n'): return IN;
			case UNIT('c','m'): return CM;
			case UNIT('m','m'): return MM;
			case UNIT('p','c'): return PC;
			case UNIT('d','d'): return DD;
			case UNIT('c','c'): return CC;
			case UNIT('s','p'): return SP;
		}
	}
	throw UnitException(string("invalid length unit: ")+unitstr);
}


string Length::unitToString (Unit unit) {
	switch (unit) {
		case PT: return "pt";
		case BP: return "bp";
		case IN: return "in";
		case CM: return "cm";
		case MM: return "mm";
		case PC: return "pc";
		case DD: return "dd";
		case CC: return "cc";
		case SP: return "sp";
	}
	// this isn't supposed to happen
	return "??";
}


void Length::set (double val, string unitstr) {
	if (unitstr.empty())
		unitstr = "pt";
	else if (unitstr.length() != 2)
		throw UnitException(string("invalid length unit: ")+unitstr);
	set(val, stringToUnit(unitstr));
}
