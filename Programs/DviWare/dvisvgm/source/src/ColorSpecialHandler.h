/*************************************************************************
** ColorSpecialHandler.h                                                **
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

#ifndef DVISVGM_COLORSPECIALHANDLER_H
#define DVISVGM_COLORSPECIALHANDLER_H

#include <stack>
#include <string>
#include <vector>
#include "Color.h"
#include "SpecialHandler.h"


class ColorSpecialHandler : public SpecialHandler
{
	public:
		bool process (const char *prefix, std::istream &is, SpecialActions *actions);
		static Color readColor (std::istream &is);
		static Color readColor (const std::string &model, std::istream &is);
		const char* name () const   {return "color";}
		const char* info () const   {return "complete support of color specials";}
		const char** prefixes () const;

	private:
		std::stack<Color> _colorStack;
};

#endif
