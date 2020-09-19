/*************************************************************************
** ColorSpecialHandler.hpp                                              **
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

#ifndef COLORSPECIALHANDLER_HPP
#define COLORSPECIALHANDLER_HPP

#include <stack>
#include <string>
#include <vector>
#include "Color.hpp"
#include "SpecialHandler.hpp"


class ColorSpecialHandler : public SpecialHandler {
	public:
		bool process (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		static Color readColor (std::istream &is);
		static Color readColor (const std::string &model, std::istream &is);
		const char* name () const override {return "color";}
		const char* info () const override {return "complete support of color specials";}
		std::vector<const char*> prefixes() const override;

	private:
		std::stack<Color> _colorStack;
};

#endif
