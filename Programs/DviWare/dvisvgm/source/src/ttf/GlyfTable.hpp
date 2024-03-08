/*************************************************************************
** GlyfTable.hpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#pragma once
#include <cmath>
#include <list>
#include <ostream>
#include <vector>
#include "TTFTable.hpp"
#include "../Pair.hpp"

namespace ttf {

struct PointInfo {
	enum State {OFFCURVE, ONCURVE, REMOVED};

	PointInfo (double x, double y, bool oncurve) : coords(std::round(x), std::round(y)), state(oncurve ? ONCURVE : OFFCURVE) {}
	PointInfo (int x, int y) : coords(x, y), state(ONCURVE) {}
	bool oncurve () const {return state == ONCURVE;}
	bool offcurve () const {return state == OFFCURVE;}
	bool removed () const {return state == REMOVED;}
	void markForRemoval () {state = REMOVED;}
	Pair<int> coords;  ///< relative coordinates to previous one in contour
	State state;
};


class Contour {
	friend class GlyfTable;
	using Iterator = std::vector<PointInfo>::iterator;
	using ConstIterator = std::vector<PointInfo>::const_iterator;
	public:
		Contour () =default;
		Contour (Contour &&contour) =default;
		Contour (std::list<Contour> &&contours);
		Contour (const Contour &contour) =delete;
		Iterator begin () {return _pointInfos.begin();}
		Iterator end () {return _pointInfos.end();}
		ConstIterator begin () const {return _pointInfos.begin();}
		ConstIterator end () const {return _pointInfos.end();}
		size_t numPoints () const {return _pointInfos.size();}
		const PointInfo& operator [] (size_t pos) const {return _pointInfos[pos];}
		bool empty () const {return _pointInfos.empty();}
		std::vector<uint8_t> glyphFlags () const;
		void reverse ();
		void reduceNumberOfPoints ();
		void append (PointInfo &&pinfo) {_pointInfos.emplace_back(std::move(pinfo));}
		void reserve (size_t n) {_pointInfos.reserve(n);}

	protected:
		void convertAbsoluteToRelative ();
#ifdef TTFDEBUG
		void writePS (std::ostream &os) const;
#endif

	private:
		std::vector<PointInfo> _pointInfos;
};


/** This class provides the functions required to write the glyf data table of a TTF/OTF font.
 *  https://www.microsoft.com/typography/otspec/glyf.htm */
class GlyfTable : public TTFTable {
	public:
		uint32_t tag () const override {return name2id("glyf");}
		void write (std::ostream &os) const override;

	protected:
		std::vector<Contour> computeContours () const;
		std::list<Contour> computeContours (uint32_t charcode) const;
		static void computeBbox (const std::list<Contour> &contours, int &xmin, int &ymin, int &xmax, int &ymax);
		size_t writeGlyphContours (std::ostream &os, uint32_t charcode) const;
//		Contour getNotDefContour () const;
#ifdef TTFDEBUG
		void writePS (std::ostream &os, const std::list<Contour> &contours, uint32_t charcode) const;
#endif
};

} // namespace ttf