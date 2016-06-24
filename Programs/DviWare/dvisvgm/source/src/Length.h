/*************************************************************************
** Length.h                                                             **
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

#ifndef DVISVGM_LENGTH_H
#define DVISVGM_LENGTH_H

#include <string>
#include "MessageException.h"

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
		enum Unit {PT, BP, CM, MM, IN, PC};

	public:
		Length () : _pt(0) {}
		Length (double val, Unit unit=PT)            {set(val, unit);}
		Length (double val, const std::string &unit) {set(val, unit);}
		Length (const std::string &len)              {set(len);}
		void set (double val, Unit unit);
		void set (double val, std::string unit);
		void set (const std::string &len);

		double pt () const {return _pt;}
		double in () const {return _pt/72.27;}
		double bp () const {return in()*72;}
		double cm () const {return in()*2.54;}
		double mm () const {return cm()*10;}
		double pc () const {return in()*12;}

	private:
		double _pt;  // length in TeX point units
};

#endif
