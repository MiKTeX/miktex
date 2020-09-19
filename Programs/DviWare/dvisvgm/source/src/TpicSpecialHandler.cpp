/*************************************************************************
** TpicSpecialHandler.cpp                                               **
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

#include <bitset>
#include <cstring>
#include <sstream>
#include <string>
#include "Color.hpp"
#include "EllipticalArc.hpp"
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "GraphicsPath.hpp"
#include "SpecialActions.hpp"
#include "SVGTree.hpp"
#include "TpicSpecialHandler.hpp"
#include "utility.hpp"

using namespace std;


TpicSpecialHandler::TpicSpecialHandler () {
	reset();
}


void TpicSpecialHandler::dviEndPage (unsigned pageno, SpecialActions&) {
	reset();
}


void TpicSpecialHandler::reset () {
	_points.clear();
	_penwidth = 1.0;
	_grayLevel = -1.0; // no fill
}


/** Returns the current color used to fill closed paths or shapes.
 *  The TPIC documentation only specifies grayscale colors set by a floating point value
 *  in the range of 0 (white) and 1 (black). Most DVI drivers extend the color support
 *  by taking color specials into account: The grayscale colors are only applied if
 *  the DVI color is set to black. All other colors are directly used as fill color
 *  without evaluating the TPIC gray level. */
Color TpicSpecialHandler::fillColor (bool grayOnly) const {
	if (_grayLevel >= 0 && (_dviColor == Color::BLACK || grayOnly))
		return Color::WHITE*(1.0-_grayLevel);
	return _dviColor;
}


/** Adds stroke attributes to a given element depending on the pen width.
 *  @param[in] elem element node the attributes are added to
 *  @param[in] penwidth pen with used to compute the stroke parameters
 *  @param[in] pencolor the drawing color
 *  @param[in] ddist dash/dot distance of line in PS point units (0:solid line, >0:dashed line, <0:dotted line) */
static void add_stroke_attribs (XMLElement *elem, double penwidth, Color pencolor, double ddist) {
	if (penwidth > 0) {  // attributes actually required?
		elem->addAttribute("stroke", pencolor.svgColorString());
		elem->addAttribute("stroke-width", XMLString(penwidth));
		if (ddist > 0)
			elem->addAttribute("stroke-dasharray", XMLString(ddist));
		else if (ddist < 0)
			elem->addAttribute("stroke-dasharray", XMLString(penwidth) + ' ' + XMLString(-ddist));
	}
}


static unique_ptr<XMLElement> create_ellipse_element (double cx, double cy, double rx, double ry) {
	bool is_circle = (rx == ry);
	auto elem = util::make_unique<XMLElement>(is_circle ? "circle" : "ellipse");
	elem->addAttribute("cx", XMLString(cx));
	elem->addAttribute("cy", XMLString(cy));
	if (is_circle)
		elem->addAttribute("r", XMLString(rx));
	else {
		elem->addAttribute("rx", XMLString(rx));
		elem->addAttribute("ry", XMLString(ry));
	}
	return elem;
}


/** Creates SVG elements that draw lines through the recorded points.
 *  @param[in] ddist dash/dot distance of line in PS point units (0:solid line, >0:dashed line, <0:dotted line)
 *  @param[in] actions object providing the actions that can be performed by the SpecialHandler */
void TpicSpecialHandler::drawLines (double ddist, SpecialActions &actions) {
	if (!_points.empty() && (_penwidth > 0 || _grayLevel >= 0) && !actions.outputLocked()) {
		unique_ptr<XMLElement> elem;
		if (_points.size() == 1) {
			const DPair &p = _points.back();
			elem = create_ellipse_element(p.x()+actions.getX(), p.y()+actions.getY(), _penwidth/2.0, _penwidth/2.0);
			actions.embed(p, _penwidth/2.0);
		}
		else {
			if (_points.size() == 2 || (_grayLevel < 0 && _points.front() != _points.back())) {
				elem = util::make_unique<XMLElement>("polyline");
				elem->addAttribute("fill", "none");
				elem->addAttribute("stroke-linecap", "round");
			}
			else {
				while (_points.front() == _points.back())
					_points.pop_back();
				elem = util::make_unique<XMLElement>("polygon");
				elem->addAttribute("fill", _grayLevel < 0 ? "none" : fillColor(false).svgColorString());
			}
			ostringstream oss;
			for (auto it=_points.begin(); it != _points.end(); ++it) {
				if (it != _points.begin())
					oss << ' ';
				double x = it->x()+actions.getX();
				double y = it->y()+actions.getY();
				oss << XMLString(x) << ',' << XMLString(y);
				actions.embed(DPair(x, y));
			}
			elem->addAttribute("points", oss.str());
			add_stroke_attribs(elem.get(), _penwidth, Color::BLACK, ddist);
		}
		actions.svgTree().appendToPage(std::move(elem));
	}
	reset();
}


/** Stroke a quadratic spline through the midpoints of the lines defined by
 *  the previously recorded points. The spline starts	with a straight line
 *  from the first point to the mid-point of the first line.  The spline ends
 *  with a straight line from the mid-point of the last line to the last point.
 *  If ddist=0, the spline is stroked solid. Otherwise ddist denotes the length
 *  of the dashes and the gaps inbetween.
 *  @param[in] ddist length of dashes and gaps
 *  @param[in] actions object providing the actions that can be performed by the SpecialHandler */
void TpicSpecialHandler::drawSplines (double ddist, SpecialActions &actions) {
	if (!_points.empty() && _penwidth > 0 && !actions.outputLocked()) {
		const size_t numPoints = _points.size();
		if (numPoints < 3) {
			_grayLevel = -1;
			drawLines(ddist, actions);
		}
		else {
			DPair p(actions.getX(), actions.getY());
			GraphicsPath<double> path;
			path.moveto(p+_points[0]);
			DPair mid = p+_points[0]+(_points[1]-_points[0])/2.0;
			path.lineto(mid);
			actions.embed(p+_points[0]);
			for (size_t i=1; i < numPoints-1; i++) {
				const DPair p0 = p+_points[i-1];
				const DPair p1 = p+_points[i];
				const DPair p2 = p+_points[i+1];
				mid = p1+(p2-p1)/2.0;
				path.quadto(p1, mid);
				actions.embed(mid);
				actions.embed((p0+p1*6.0+p2)/8.0, _penwidth);
			}
			if (_points[0] == _points[numPoints-1])  // closed path?
				path.closepath();
			else {
				path.lineto(p+_points[numPoints-1]);
				actions.embed(p+_points[numPoints-1]);
			}
			auto pathElem = util::make_unique<XMLElement>("path");
			pathElem->addAttribute("fill", "none");
			ostringstream oss;
			path.writeSVG(oss, SVGTree::RELATIVE_PATH_CMDS);
			pathElem->addAttribute("d", oss.str());
			add_stroke_attribs(pathElem.get(), _penwidth, _dviColor, ddist);
			actions.svgTree().appendToPage(std::move(pathElem));
		}
	}
	reset();
}


/** Draws an elliptical arc.
 *  @param[in] cx x-coordinate of arc center relative to current DVI position
 *  @param[in] cy y-coordinate of arc center relative to current DVI position
 *  @param[in] rx length of horizontal semi-axis
 *  @param[in] ry length of vertical semi-axis
 *  @param[in] angle1 starting angle (clockwise) relative to x-axis
 *  @param[in] angle2 ending angle (clockwise) relative to x-axis
 *  @param[in] actions object providing the actions that can be performed by the SpecialHandler */
void TpicSpecialHandler::drawArc (double cx, double cy, double rx, double ry, double angle1, double angle2, SpecialActions &actions) {
	if ((_penwidth > 0 || _grayLevel >= 0) && !actions.outputLocked()) {
		cx += actions.getX();
		cy += actions.getY();
		unique_ptr<XMLElement> elem;
		bool closed=true;
		if (abs(angle2-angle1) >= math::TWO_PI) // closed ellipse?
			elem = create_ellipse_element(cx, cy, rx, ry);
		else {
			EllipticalArc arc(DPair(cx, cy), rx, ry, 0, -angle1, math::normalize_0_2pi(angle2-angle1));
			GraphicsPath<double> path;
			path.moveto(arc.startPoint());
			path.arcto(rx, ry, 0, arc.largeArc(), arc.sweepPositive(), arc.endPoint());
			if (_grayLevel >= 0)
				path.closepath();
			else
				closed = false;
			elem = util::make_unique<XMLElement>("path");
			ostringstream oss;
			path.writeSVG(oss, SVGTree::RELATIVE_PATH_CMDS);
			elem->addAttribute("d", oss.str());
		}
		if (_penwidth > 0) {
			elem->addAttribute("stroke-width", _penwidth);
			elem->addAttribute("stroke", actions.getColor().svgColorString());
			if (!closed)
				elem->addAttribute("stroke-linecap", "round");
		}
		elem->addAttribute("fill", _grayLevel < 0 ? "none" : fillColor(true).svgColorString());
		actions.svgTree().appendToPage(std::move(elem));
		double pw = _penwidth/2.0;
		actions.embed(BoundingBox(cx-rx-pw, cy-ry-pw, cx+rx+pw, cy+ry+pw));
	}
	reset();
}


/** Computes the gray level based on the ratio of set bits to the total
 *  number of bits of a given hex value.
 *  @param[in] hexstr a sequence of hexadecimal digits
 *  @return the computed gray level [0-1] */
static double bit_sequence_to_gray (const string &hexstr) {
	if (hexstr.empty())
		return 1.0;
	int setbits=0;   // number of bits set
	int totalbits=0; // number of bits processed
	constexpr int CHUNKBITS = 8*sizeof(unsigned long long);
	for (size_t pos=0; pos < hexstr.length(); pos+=CHUNKBITS/4) {
		size_t digitcount;  // number of hex digits processed
		unsigned long long val = stoull(hexstr.substr(pos, CHUNKBITS/4), &digitcount, 16);
		setbits += bitset<CHUNKBITS>(val).count();
		totalbits += 4*digitcount;
	}
	return totalbits == 0 ? 1.0 : 1.0-double(setbits)/double(totalbits);
}


/** Returns a unique integer for a TPIC command (consisting of two letters). */
constexpr int cmd_id (const char *cmd) {
	return (cmd[0] << 8) | cmd[1];
};


bool TpicSpecialHandler::process (const string &prefix, istream &is, SpecialActions &actions) {
	if (prefix.length() != 2)
		return false;
	_dviColor = actions.getColor();
	const double mi2bp=0.072; // factor for milli-inch to PS points
	StreamInputBuffer ib(is);
	BufferInputReader ir(ib);
	switch (cmd_id(prefix.c_str())) {
		case cmd_id("pn"): // set pen width in milli-inches
			_penwidth = max(0.0, ir.getDouble()*mi2bp);
			break;
		case cmd_id("bk"): // set fill color to black
			_grayLevel = 1;
			break;
		case cmd_id("wh"): // set fill color to white
			_grayLevel = 0;
			break;
		case cmd_id("sh"): // set fill color to given gray level (0=white, 1=black)
			ir.skipSpace();
			_grayLevel = ir.eof() ? 0.5 : max(0.0, min(1.0, ir.getDouble()));
			break;
		case cmd_id("tx"): { // set fill color depending on a sequence of bits (given as hex value)
			string hexstr;
			while (!ir.eof()) {
				ir.skipSpace();
				int c = ir.get();
				if (isxdigit(c))
					hexstr += static_cast<char>(tolower(c));
				else
					break;
			}
			_grayLevel = bit_sequence_to_gray(hexstr);
			break;
		}
		case cmd_id("pa"): { // add point to path
			double x = ir.getDouble()*mi2bp;
			double y = ir.getDouble()*mi2bp;
			_points.emplace_back(DPair(x,y));
			break;
		}
		case cmd_id("fp"): // draw solid lines through recorded points; close and fill path if fill color was defined
			drawLines(0, actions);
			break;
		case cmd_id("ip"): // don't draw outlines but close the recorded path and fill the resulting polygon
			_penwidth = 0;
			drawLines(0, actions);
			break;
		case cmd_id("da"): // as fp but draw dashed lines
			drawLines(ir.getDouble()*72, actions);
			break;
		case cmd_id("dt"): // as fp but draw dotted lines
			drawLines(-ir.getDouble()*72, actions);
			break;
		case cmd_id("sp"): { // draw quadratic splines through recorded points
			double ddist = ir.getDouble()*72;
			drawSplines(ddist, actions);
			break;
		}
		case cmd_id("ar"): { // draw elliptical arc
			double cx = ir.getDouble()*mi2bp;
			double cy = ir.getDouble()*mi2bp;
			double rx = ir.getDouble()*mi2bp;
			double ry = ir.getDouble()*mi2bp;
			double a1 = ir.getDouble();
			double a2 = ir.getDouble();
			drawArc(cx, cy, rx, ry, a1, a2, actions);
			break;
		}
		case cmd_id("ia"): { // fill elliptical arc
			double cx = ir.getDouble()*mi2bp;
			double cy = ir.getDouble()*mi2bp;
			double rx = ir.getDouble()*mi2bp;
			double ry = ir.getDouble()*mi2bp;
			double a1 = ir.getDouble();
			double a2 = ir.getDouble();
			_penwidth = 0;
			drawArc(cx, cy, rx, ry, a1, a2, actions);
			break;
		}
		default:
			return false;
	}
	return true;
}


vector<const char*> TpicSpecialHandler::prefixes() const {
	vector<const char*> pfx {"ar", "bk", "da", "dt", "fp", "ia", "ip", "pa", "pn", "sh", "sp", "tx", "wh"};
	return pfx;
}
