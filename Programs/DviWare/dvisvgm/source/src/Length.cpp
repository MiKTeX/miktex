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

#include <config.h>
#include <sstream>
#include "InputReader.h"
#include "Length.h"

#define UNIT(c1,c2) ((c1 << 8)|c2)

using namespace std;


void Length::set (const string &len) {
	switch (len.length()) {
		case 0:
			_pt = 0;
			break;
		case 1:
			if (isdigit(len[0]))
				_pt = len[0] - '0';
			else
				throw UnitException("invalid length: "+len);
			break;
		default:
			istringstream iss(len);
			StreamInputReader ir(iss);
			double val;
			if (!ir.parseDouble(val))
				throw UnitException("invalid length: "+len);
			string unit = ir.getWord();
			set(val, unit);
	}
}


void Length::set (double val, Unit unit) {
	switch (unit) {
		case PT: _pt = val; break;
		case BP: _pt = val*72.27/72; break;
		case IN: _pt = val*72.27; break;
		case CM: _pt = val/2.54*72.27; break;
		case MM: _pt = val/25.4*72.27; break;
		case PC: _pt = val/12*72.27; break;
	}
}


void Length::set (double val, string unitstr) {
	if (unitstr.empty())
		unitstr = "pt";
	else if (unitstr.length() != 2)
		throw UnitException("invalid length unit: "+unitstr);

	Unit unit;
	switch (UNIT(unitstr[0], unitstr[1])) {
		case UNIT('p','t'): unit = PT; break;
		case UNIT('b','p'): unit = BP; break;
		case UNIT('i','n'): unit = IN; break;
		case UNIT('c','m'): unit = CM; break;
		case UNIT('m','m'): unit = MM; break;
		case UNIT('p','c'): unit = PC; break;
		default:
			throw UnitException("invalid length unit: "+unitstr);
	}
	set(val, unit);
}

