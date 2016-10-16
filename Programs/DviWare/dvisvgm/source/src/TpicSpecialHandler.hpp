/*************************************************************************
** TpicSpecialHandler.hpp                                               **
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

#ifndef TPICSPECIALHANDLER_HPP
#define TPICSPECIALHANDLER_HPP

#include <list>
#include "Pair.hpp"
#include "SpecialHandler.hpp"

class TpicSpecialHandler : public SpecialHandler, public DVIEndPageListener
{
	public:
		TpicSpecialHandler ();
		const char* info () const override {return "TPIC specials";}
		const char* name () const override {return "tpic";}
		const char** prefixes () const override;
		bool process (const char *prefix, std::istream &is, SpecialActions &actions) override;

	protected:
		void dviEndPage (unsigned pageno, SpecialActions &actions) override;
		void reset ();
		void drawLines (bool stroke, bool fill, double ddist, SpecialActions &actions);
		void drawSplines (double ddist, SpecialActions &actions);
		void drawArc (double cx, double cy, double rx, double ry, double angle1, double angle2, SpecialActions &actions);

	private:
		double _penwidth; ///< pen width in PS point units
		double _fill;     ///< fill intensity [0,1]; if < 0, we don't fill anything
		std::vector<DPair> _points;
};

#endif
