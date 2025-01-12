/*************************************************************************
** GlyfTable.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <algorithm>
#include <limits>
#include <cmath>
#include <numeric>
#include <sstream>
#include "GlyfTable.hpp"
#include "TTFWriter.hpp"
#include "../Bezier.hpp"
#include "../Font.hpp"

using namespace std;
using namespace ttf;

/** Creates a contour by concatenating several contours given in a list. */
Contour::Contour (std::list<Contour> &&contours) {
	if (_pointInfos.empty() && !contours.empty()) {
		_pointInfos = std::move(contours.front()._pointInfos);
		contours.pop_front();
	}
	for (auto it=contours.begin(); it != contours.end(); it=contours.erase(it))
		std::move(it->begin(), it->end(), std::back_inserter(_pointInfos));
}


void Contour::reverse () {
	std::reverse(_pointInfos.begin(), _pointInfos.end());
}


/** Converts the absolute coordinates of a Contour to relative ones. */
void Contour::convertAbsoluteToRelative () {
	Pair<int> prevPoint;
	for (PointInfo &info : _pointInfos) {
		Pair<int> p = info.coords;
		info.coords -= prevPoint;
		prevPoint = p;
	}
}


/** Returns an iterator pointing to the next PointInfo object in a vector that isn't marked for removal.
 *  If there's no such object, the function returns infos.end().
 *  @param[in] it current position
 *  @param[in] infos vector the iterator belongs to */
static vector<PointInfo>::iterator next (vector<PointInfo>::iterator it, const vector<PointInfo> &infos) {
	do
		++it;
	while (it != infos.end() && it->removed());
	return it;
}


/** Reduces the number of contour points without affecting the appearance significantly. */
void Contour::reduceNumberOfPoints () {
	if (_pointInfos.size() > 2) {
		for (auto it1 = _pointInfos.begin(); it1+2 != _pointInfos.end(); ++it1) {
			auto it2 = it1+1;
			auto it3 = it2+1;
			bool remove=false;
			if (it1->oncurve() && it2->offcurve() && it3->oncurve()) {
				// remove off-curve points that are close to an on-curve point
				Pair<int> diff12 = it1->coords - it2->coords;
				Pair<int> diff23 = it2->coords - it3->coords;
				int sqrdist12 = diff12.x()*diff12.x() + diff12.y()*diff12.y();
				int sqrdist23 = diff23.x()*diff23.x() + diff23.y()*diff23.y();
				remove = (sqrdist12 < 4 || sqrdist23 < 4);
				if (!remove) {
					// remove in-between off-curve points that are very close to
					// the line through the two adjacent on-curve points (curve height <= 0.5)
					// distance(line(p1,p3), p2) = |det(p1-p3, p1-p2)| / |p1-p3|
					Pair<int> diff13 = it1->coords - it3->coords;
					DPair extremum = QuadBezier(it1->coords, it2->coords, it3->coords).valueAt(0.5);
					int sqrdist13 = diff13.x()*diff13.x() + diff13.y()*diff13.y();
					double det1312 = det(DPair(diff13), DPair(it3->coords)-extremum);
					remove = (sqrdist13 == 0 || det1312/sqrdist13*det1312 < 0.25);
				}
			}
			else if (it2->offcurve() || (it1->oncurve() && it3->oncurve())) {
				// remove in-between points located on same line
				Pair<int> diff13 = it3->coords - it1->coords;
				Pair<int> diff12 = it2->coords - it1->coords;
				int sqrdist13 = diff13.x()*diff13.x() + diff13.y()*diff13.y();
				double det1312 = det(diff13, diff12);
				remove = (sqrdist13 == 0 || det1312/sqrdist13*det1312 < 1);
			}
			else if (it1->offcurve() && it2->oncurve() && it3->offcurve()) {
				// remove on-curve points located (almost) halfway between two off-curve points
				Pair<int> mid = DPair(it1->coords + it3->coords)/2.0;
				Pair<int> vec = mid - it2->coords;
				double dist2 = vec.x()*vec.x() + vec.y()*vec.y();
				remove = (dist2 < 2.25);  // remove point if dist < 1.5
			}
			if (remove)
				it2->markForRemoval();
		}
	}
	// remove duplicate points, prefer control (off-curve) points
	for (auto it1 = _pointInfos.begin(); it1 != _pointInfos.end(); it1=next(it1, _pointInfos)) {
		auto it2 = next(it1, _pointInfos);
		if (it2 == _pointInfos.end())
			it2 = _pointInfos.begin();
		if (it1->oncurve() && it2->oncurve() && (it1->coords-it2->coords).length() < 6) {
			auto i1=it1, i2=it2;
			if (i1 == _pointInfos.begin())
				swap(i1, i2);
			i2->coords = DPair(i1->coords+i2->coords)/2.0;
			i1->markForRemoval();
		}
		else if (it1->coords == it2->coords) {
			if (it1->offcurve() || it2 == _pointInfos.begin())
				it1->markForRemoval();
			else
				it2->markForRemoval();
		}
	}
	// actually remove marked points from vector
	_pointInfos.erase(
		remove_if(_pointInfos.begin(), _pointInfos.end(), [](const PointInfo &pi) {
			return pi.removed();
		}),
		_pointInfos.end());
}


/** Bits of the "Simple Glyph Flags". See section "Simple Glyph Description" of glyf table documentation. */
enum : uint8_t {
	ON_CURVE_PT = 1,   ///< if set, corresponing point is on the curve, otherwise off the curve (= control point)
	X_SHORT = 2,       ///< if set, the corresponding x-coordinate is 1 byte long, otherwise it's two bytes long
	Y_SHORT = 4,       ///< if set, the corresponding y-coordinate is 1 byte long, otherwise it's two bytes long
	REPEAT_FLAG = 8,   ///< if set, the next byte specifies the number of additional times this flag is to be repeated
	X_IS_SAME_OR_POSITIVE_SHORT = 16, ///< if X_SHORT set: indicates if value >0, else if x-coord equals prev. one
	Y_IS_SAME_OR_POSITIVE_SHORT = 32  ///< if Y_SHORT set: indicates if value >0, else if y-coord equals prev. one
};


/** Computes the "simple glyph flags" (except the repeat flag) for a given contour.
 *  The function expects the contour points to be relative to the respective preceding point.
 *  In order to simplify checking the equality of two flag bytes, which requires to
 *  exclude the repeat flag, this function does not set the repeat flag. */
vector<uint8_t> Contour::glyphFlags () const {
	vector<uint8_t> flags(_pointInfos.size());
	for (size_t i=0; i < _pointInfos.size(); i++) {
		flags[i] = _pointInfos[i].oncurve() ? ON_CURVE_PT : 0;
		if (_pointInfos[i].coords.x() == 0)
			flags[i] |= X_IS_SAME_OR_POSITIVE_SHORT;
		else if (abs(_pointInfos[i].coords.x()) <= 0xff) {  // x-coordinate 1 byte long?
			flags[i] |= X_SHORT;
			if (_pointInfos[i].coords.x() > 0)          // positive x-coordinate?
				flags[i] |= X_IS_SAME_OR_POSITIVE_SHORT;
		}
		if (_pointInfos[i].coords.y() == 0)
			flags[i] |= Y_IS_SAME_OR_POSITIVE_SHORT;
		else if (abs(_pointInfos[i].coords.y()) <= 0xff) {  // y-coordinate 1 byte long?
			flags[i] |= Y_SHORT;
			if (_pointInfos[i].coords.y() > 0)          // positive y-coordinate?
				flags[i] |= Y_IS_SAME_OR_POSITIVE_SHORT;
		}
	}
	return flags;
}


////////////////////////////////////////////////////////////////////////////////////////////////


/** Writes the contours describing the glyphs to a given stream.
 *  The glyphs must be ordered by the local glyph index used in the generated TTF font.
 *  In order to get compact mapping tables, we ensure that a greater glyph index also
 *  indicates a greater Unicode point (idx1 > idx2 <=> cp1 > cp2). */
void GlyfTable::write (ostream &os) const {
	// add offset for .notdef character
	size_t offset = 0;
	ttfWriter()->addGlyphOffset(0);
	ttfWriter()->updateGlobalBbox(0, 0, 0, 0, 0);
	// iterate in ascending Unicode point order over all characters to be added to the TTF
	for (auto cp2charcode : ttfWriter()->getUnicodeCharMap()) {
		ttfWriter()->addGlyphOffset(offset); // update loca table
		offset += writeGlyphContours(os, cp2charcode.second);
	}
	ttfWriter()->addGlyphOffset(offset);  // add extra offset after last valid one
}


/** Writes the contour data of a single glyph to a given output stream.
 *  @param[in] os stream to write to
 *  @param[in] charcode character code of glyph in current font
 *  @return number of bytes written */
size_t GlyfTable::writeGlyphContours (ostream &os, uint32_t charcode) const {
	list<Contour> contours = computeContours(charcode);
	if (contours.empty()) {
		ttfWriter()->updateGlobalBbox(charcode, 0, 0, 0, 0);
		return 0;
	}
	ttfWriter()->updateContourInfo(contours.size(),
		std::accumulate(contours.begin(), contours.end(), size_t(0), [](size_t sum, const Contour &contour) {
			return sum + contour.numPoints();
		}));
	auto offset = os.tellp();
	// glyph header
	writeInt16(os, int16_t(contours.size()));  // numberOfContours

	int xmin, xmax, ymin, ymax;
	computeBbox(contours, xmin, ymin, xmax, ymax);
	ttfWriter()->updateGlobalBbox(charcode, xmin, ymin, xmax, ymax);
	writeInt16(os, xmin);
	writeInt16(os, ymin);
	writeInt16(os, xmax);
	writeInt16(os, ymax);

	// write indices of endpoints of each contour
	size_t endPtsOfContours = 0;
	for (const Contour &contour : contours) {
		endPtsOfContours += contour.numPoints();
		writeUInt16(os, endPtsOfContours-1);
	}
	writeUInt16(os, 0);  // instructionLength

	Contour combinedContour(std::move(contours));
	combinedContour.convertAbsoluteToRelative();

	// write contour flags
	vector<uint8_t> flags = combinedContour.glyphFlags();
	for (auto first=flags.begin(); first != flags.end(); ++first) {
		auto last = std::find_if(first+1, flags.end(), [&first](uint8_t flagbyte) {
			return flagbyte != *first;
		});
		if (--last == first)   // run-length == 0?
			writeUInt8(os, *first);
		else {
			writeUInt8(os, (*first) | REPEAT_FLAG);  // write flag only once and
			writeUInt8(os, last-first);   // append number of additional repeats
			first = last;
		}
	}
	// write x-coordinates
	for (size_t i=0; i < combinedContour.numPoints(); i++) {
		if (flags[i] & X_SHORT)  // short (one byte) x coordinate?
			writeUInt8(os, abs(combinedContour[i].coords.x()));
		else if ((flags[i] & X_IS_SAME_OR_POSITIVE_SHORT) == 0)  // does long x-coordinate differ from previous one?
			writeInt16(os, int16_t(combinedContour[i].coords.x()));
	}
	// write y-coordinates
	for (size_t i=0; i < combinedContour.numPoints(); i++) {
		if (flags[i] & Y_SHORT)  // short (one byte) y coordinate?
			writeUInt8(os, abs(combinedContour[i].coords.y()));
		else if ((flags[i] & Y_IS_SAME_OR_POSITIVE_SHORT) == 0)  // does long y-coordinate differ from previous one?
			writeInt16(os, int16_t(combinedContour[i].coords.y()));
	}
	// ensure that the number of glyph data bytes is a multiple of 4 (pad at dword boundary)
	for (int i = (1 + ~(os.tellp()-offset)) & 3; i > 0; i--)
		writeUInt8(os, 0);
	return os.tellp()-offset;
}


/** Computes the absolute global bounds of multiple contours.
 *  @param[in] contours point data with absolute coordinates */
void GlyfTable::computeBbox (const list<Contour> &contours, int &xmin, int &ymin, int &xmax, int &ymax) {
	if (contours.empty())
		xmin = xmax = ymin = ymax = 0;
	else {
		xmin = ymin = numeric_limits<int16_t>::max();
		xmax = ymax = numeric_limits<int16_t>::min();
		for (const Contour &contour : contours) {
			for (const PointInfo &info : contour) {
				const auto &p = info.coords;
				xmin = min(xmin, p.x());
				xmax = max(xmax, p.x());
				ymin = min(ymin, p.y());
				ymax = max(ymax, p.y());
			}
		}
	}
}


static Glyph::Point to_point (const DPair &p) {
	return Glyph::Point(int(std::round(p.x())), int(std::round(p.y())));
}


struct GlyphToContourActions : Glyph::IterationActions {
	GlyphToContourActions (list<Contour> &cnts, double s) : contours(cnts), scale(s) {}

	void moveto (const Glyph::Point &p) override {
		contours.emplace_back(Contour());
		add(p, true);
	}

	void lineto (const Glyph::Point &p) override {
		if (contours.empty())
			moveto(currentPoint());  // (0, 0)
		if (p != currentPoint())
			add(p, true);
	}

	void quadto (const Glyph::Point &p1, const Glyph::Point &p2) override {
		if (contours.empty())
			moveto(currentPoint());  // (0, 0)
		// if the maximum distance between the curve and the line through p0 and p2 is < 0.5,
		// treat the curve as a line
		DPair extremum = QuadBezier(currentPoint(), p1, p2).valueAt(0.5);
		double height = det(DPair(currentPoint()-p2), DPair(currentPoint())-extremum)/(currentPoint()-p2).length();
		if (p1 != currentPoint() && abs(height) >= 0.5 && p2 != currentPoint())
			add(p1, false);
		if (p2 != currentPoint())
			add(p2, true);
	}

	void cubicto (const Glyph::Point &p1, const Glyph::Point &p2, const Glyph::Point &p3) override {
		// approximate cubic Bézier curve with a sequence of quadratic ones
		CubicBezier cubic(
			DPair(currentPoint().x(), currentPoint().y()),
			DPair(p1.x(), p1.y()),
			DPair(p2.x(), p2.y()),
			DPair(p3.x(), p3.y())
		);
		vector<QuadBezier> quadBeziers = cubic.toQuadBeziers(0.8);
		for (const QuadBezier &quad : quadBeziers)
			quadto(to_point(quad.point(1)), to_point(quad.point(2)));
	}

	void add (const Glyph::Point &p, bool oncurve) {
		contours.back().append(PointInfo(scale*p.x(), scale*p.y(), oncurve));
	}

	list<Contour> &contours;
	double scale;
};


/** Returns a sequence of contours describing the glyph of a given character.
 *  The contour points are stored with absolute coordinates. */
list<Contour> GlyfTable::computeContours (uint32_t charcode) const {
	list<Contour> contours;
	Glyph glyph;
	if (ttfWriter()->getFont().getGlyph(int(charcode), glyph, ttfWriter()->getTracerCallback())) {
		if (!glyph.empty()) {
			GlyphToContourActions actions(contours, ttfWriter()->unitsPerEmFactor());
			glyph.iterate(actions, false);
			for (auto it = contours.begin(); it != contours.end();) {
				it->reverse();  // TTF contours must be clockwise-oriented
				it->reduceNumberOfPoints();
				if (it->numPoints() < 3)
					it = contours.erase(it);
				else
					++it;
			}
		}
#ifdef TTFDEBUG
		if (TTFWriter::CREATE_PS_GLYPH_OUTLINES) {
			string fontname = ttfWriter()->getFont().name();
			ostringstream oss;
			oss << "g-" << ttfWriter()->getFont().name() << '-';
			oss << setw(4) << setfill('0') << charcode;
			oss << ".eps";
			ofstream ofs(oss.str());
			writePS(ofs, contours, charcode);
		}
#endif
	}
	return contours;
}


#if 0
/** Creates a contour for the .notdef character. */
Contour GlyfTable::getNotDefContour () const {
	Contour contour;
	const PhysicalFont &font = ttfWriter()->getFont();
	double extend = font.style() ? font.style()->extend : 1;
	double scale = ttfWriter()->unitsPerEmFactor();
	int base = int(round(scale*extend*(font.ascent()+font.descent())));
	Pair<int> pmin(base/30, 0);
	Pair<int> pmax(3*base/5, 2*base/3);
	contour.append(PointInfo(pmin.x(), pmin.y()));
	contour.append(PointInfo(pmin.x(), pmax.y()));
	contour.append(PointInfo(pmax.x(), pmax.y()));
	contour.append(PointInfo(pmax.x(), pmin.y()));
	return contour;
}
#endif


#ifdef TTFDEBUG
/** Creates PostScript code from a list of contours that describe the outline of a single glyph.
 *  This is just required for analysis and debugging purposes. */
void GlyfTable::writePS (ostream &os, const list<Contour> &contours, uint32_t charcode) const {
	os << "%!PS-Adobe-3.0 EPSF-3.0\n"
		<< "%%BoundingBox: (atend)\n"
		<< "/dot {moveto gsave 5 setlinewidth 1 setlinecap currentpoint lineto stroke grestore} bind def\n"
		<< "/oncurvedot {dot} bind def\n"
		<< "/offcurvedot {gsave 0 1 0 setrgbcolor dot grestore} bind def\n"
		<< "/Helvetica findfont 8 scalefont setfont\n\n";
	auto &font = ttfWriter()->getFont();
	os << "% Font: " << font.name() << " at " << font.designSize() << "pt, " << font.unitsPerEm() << " units per em\n"
		<< "% Glyph: " << charcode << "\n";
	Pair<int> llp(numeric_limits<int>::max(), numeric_limits<int>::max());
	Pair<int> urp(numeric_limits<int>::min(), numeric_limits<int>::min());
	size_t count=0, numPoints=0;
	for (const Contour &contour : contours) {
		os << "\n% contour " << ++count << ", " << contour.numPoints() << " points\n";
		contour.writePS(os);
		numPoints += contour.numPoints();
	}
	int minx, miny, maxx, maxy;
	computeBbox(contours, minx, miny, maxx, maxy);
	os << minx << " " << maxy+30
		<< " moveto (Glyph " << charcode << " of font " << font.name() << ", "
		<< contours.size() << " contour" << (contours.size() > 1 ? "s" : "") << ", " << numPoints << " points) show\n";
	os << "%%Trailer\n"
		<< "%%BoundingBox: " << minx-10 << " " << miny-10 << " " << maxx+10 << " " << maxy+50 << "\n";
}


static ostream& write (ostream &os, const Pair<int> &p) {
	return os << p.x() << " " << p.y();
}

template <typename ...Ts>
static ostream& write (ostream &os, const Pair<int> &p, const Ts& ...args) {
	write(os, p) << " ";
	return write(os, args...);
}


void Contour::writePS (ostream &os) const {
	if (_pointInfos.empty())
		return;
	auto p0 = _pointInfos[0].coords;
	write(os, p0) << " moveto\n";
	for (size_t i=1; i <= _pointInfos.size(); i++) {
		size_t imod = i % _pointInfos.size();
		auto pi1 = _pointInfos[imod];
		if (pi1.oncurve() && i == _pointInfos.size())
			os << "closepath\n";
		else
			write(os, pi1.coords) << " ";
		if (pi1.oncurve()) {
			if (i < _pointInfos.size()) {
				auto v = pi1.coords-p0;
				os << " lineto  % " << i << "; len=" << v.length()
					<< "; " << math::rad2deg(atan2(v.y(), v.x())) << "°\n";
			}
			p0 = pi1.coords;
		}
		else {
			auto p1 = pi1.coords;
			auto pi2 = _pointInfos[(i+1) % _pointInfos.size()];
			auto p2 = pi2.oncurve() ? pi2.coords : (p1+pi2.coords)/2;
			DPair v1 = p1-p0, v2 = p1-p2;
			Pair<int> c1 = DPair(p0) + round(v1*2.0/3.0);
			Pair<int> c2 = DPair(p2) + round(v2*2.0/3.0);
			QuadBezier bezier(p0, p1, p2);
			double cordlen = (p2-p0).length();
			double arclen = bezier.arclen();
			DPair extremum = bezier.valueAt(0.5);
			double height = det(DPair(p0-p2), DPair(p0)-extremum)/(p0-p2).length();
			write(os, c1, c2, p2) << " curveto"
				<< "  % " << i
				<< "; (" << math::rad2deg(atan2(v1.y(), v1.x())) << "°," << math::rad2deg(atan2(v2.y(), v2.x())) << "°)"
				<< "; cordlen=" << cordlen
				<< "; arclen=" << arclen
				<< "; quot=" << cordlen/arclen
				<< "; diff=" << arclen-cordlen
				<< "; height=" << height
				<< "\n";
			p0 = p2;
			if (pi2.oncurve())
				i++;
		}
	}
	os << "stroke\n";
	for (size_t i=0; i < _pointInfos.size(); i++) {
		auto p = _pointInfos[i].coords;
		write(os, p) << " 2 copy "
						 << (_pointInfos[i].oncurve() ? "on" : "off") << "curvedot "
						 << "moveto (" << i << ") show\n";
	}
}
#endif
