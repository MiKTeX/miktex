/*************************************************************************
** EmSpecialHandler.hpp                                                 **
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

#ifndef EMSPECIALHANDLER_HPP
#define EMSPECIALHANDLER_HPP

#include <unordered_map>
#include <vector>
#include "Pair.hpp"
#include "SpecialHandler.hpp"

class InputReader;
class SpecialActions;

class EmSpecialHandler : public SpecialHandler {
	struct Line {
		Line (int pp1, int pp2, char cc1, char cc2, double w) : p1(pp1), p2(pp2), c1(cc1), c2(cc2), width(w) {}
		int p1, p2;   ///< point numbers of line ends
		char c1, c2;  ///< cut type of line ends (h, v or p)
		double width; ///< line width
	};

	public:
		const char* name () const override {return "em";}
		const char* info () const override {return "line drawing statements of the emTeX special set";}
		std::vector<const char*> prefixes() const override;
		bool process (const std::string &prefix, std::istream &in, SpecialActions &actions) override;

	protected:
		void dviEndPage (unsigned pageno, SpecialActions &actions) override;
		void linewidth (InputReader &ir, SpecialActions &actions);
		void moveto (InputReader &ir, SpecialActions &actions);
		void lineto (InputReader &ir, SpecialActions &actions);
		void line (InputReader &ir, SpecialActions &actions);
		void point (InputReader &ir, SpecialActions &actions);

	private:
		std::unordered_map<int, DPair> _points; ///< points defined by special em:point
		std::vector<Line> _lines;       ///< list of lines with undefined end points
		double _linewidth=0.4*72/72.27; ///< global line width
		DPair _pos;                     ///< current position of "graphic cursor"
};

#endif
