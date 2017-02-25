/*************************************************************************
** TpicSpecialHandler.cpp                                               **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <cstring>
#include <sstream>
#include "Color.hpp"
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
	_fill = -1.0; // no fill
}


/** Creates SVG elements that draw lines through the recorded points.
 *  @param[in] stroke if true, the (out)line is drawn (in black)
 *  @param[in] fill if true, enclosed area is filled with current color
 *  @param[in] ddist dash/dot distance of line in PS point units
 *                   (0:solid line, >0:dashed line, <0:dotted line)
 *  @param[in] actions object providing the actions that can be performed by the SpecialHandler */
void TpicSpecialHandler::drawLines (bool stroke, bool fill, double ddist, SpecialActions &actions) {
	if (!_points.empty()) {
		XMLElementNode *elem=0;
		if (_points.size() == 1) {
			const DPair &p = _points.back();
			elem = new XMLElementNode("circle");
			elem->addAttribute("cx", p.x()+actions.getX());
			elem->addAttribute("cy", p.y()+actions.getY());
			elem->addAttribute("r", _penwidth/2.0);
			actions.embed(p, _penwidth/2.0);
		}
		else {
			if (_points.size() == 2 || (!fill && _points.front() != _points.back())) {
				elem = new XMLElementNode("polyline");
				elem->addAttribute("fill", "none");
				elem->addAttribute("stroke-linecap", "round");
			}
			else {
				if (_points.front() == _points.back())
					_points.pop_back();
				if (_fill < 0)
					_fill = 1;
				Color color = actions.getColor();
				color *= _fill;
				elem = new XMLElementNode("polygon");
				elem->addAttribute("fill", fill ? color.svgColorString() : "none");
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
			if (stroke) {   // draw outline?
				elem->addAttribute("stroke", "black");
				elem->addAttribute("stroke-width", XMLString(_penwidth));
			}
		}
		if (ddist > 0)
			elem->addAttribute("stroke-dasharray", XMLString(ddist));
		else if (ddist < 0)
			elem->addAttribute("stroke-dasharray", XMLString(_penwidth) + ' ' + XMLString(-ddist));
		actions.appendToPage(elem);
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
	if (_points.empty())
		return;
	const size_t size = _points.size();
	if (size < 3)
		drawLines(true, false, ddist, actions);
	else {
		DPair p(actions.getX(), actions.getY());
		GraphicsPath<double> path;
		path.moveto(p+_points[0]);
		DPair mid = p+_points[0]+(_points[1]-_points[0])/2.0;
		path.lineto(mid);
		actions.embed(p+_points[0]);
		for (size_t i=1; i < size-1; i++) {
			const DPair p0 = p+_points[i-1];
			const DPair p1 = p+_points[i];
			const DPair p2 = p+_points[i+1];
			mid = p1+(p2-p1)/2.0;
			path.conicto(p1, mid);
			actions.embed(mid);
			actions.embed((p0+p1*6.0+p2)/8.0, _penwidth);
		}
		if (_points[0] == _points[size-1])  // closed path?
			path.closepath();
		else {
			path.lineto(p+_points[size-1]);
			actions.embed(p+_points[size-1]);
		}

		Color color = actions.getColor();
		color *= _fill;
		XMLElementNode *pathElem = new XMLElementNode("path");
		if (_fill >= 0) {
			if (_points[0] != _points[size-1])
				path.closepath();
			pathElem->addAttribute("fill", color.svgColorString());
		}
		else
			pathElem->addAttribute("fill", "none");

		ostringstream oss;
		path.writeSVG(oss, SVGTree::RELATIVE_PATH_CMDS);
		pathElem->addAttribute("d", oss.str());
		pathElem->addAttribute("stroke", actions.getColor().svgColorString());
		pathElem->addAttribute("stroke-width", XMLString(_penwidth));
		if (ddist > 0)
			pathElem->addAttribute("stroke-dasharray", XMLString(ddist));
		else if (ddist < 0)
			pathElem->addAttribute("stroke-dasharray", XMLString(_penwidth) + ' ' + XMLString(-ddist));
		actions.appendToPage(pathElem);
	}
	reset();
}


/** Draws an elliptical arc.
 *  @param[in] cx x-coordinate of arc center
 *  @param[in] cy y-coordinate of arc center
 *  @param[in] rx length of horizonal semi-axis
 *  @param[in] ry length of vertical semi-axis
 *  @param[in] angle1 starting angle (clockwise) relative to x-axis
 *  @param[in] angle2 ending angle (clockwise) relative to x-axis
 *  @param[in] actions object providing the actions that can be performed by the SpecialHandler */
void TpicSpecialHandler::drawArc (double cx, double cy, double rx, double ry, double angle1, double angle2, SpecialActions &actions) {
	angle1 *= -1;
	angle2 *= -1;
	if (fabs(angle1) > math::TWO_PI) {
		int n = (int)(angle1/math::TWO_PI);
		angle1 = angle1 - n*math::TWO_PI;
		angle2 = angle2 - n*math::TWO_PI;
	}

	double x = cx + actions.getX();
	double y = cy + actions.getY();
	XMLElementNode *elem=0;
	if (fabs(angle1-angle2) >= math::TWO_PI) {  // closed ellipse?
		elem = new XMLElementNode("ellipse");
		elem->addAttribute("cx", XMLString(x));
		elem->addAttribute("cy", XMLString(y));
		elem->addAttribute("rx", XMLString(rx));
		elem->addAttribute("ry", XMLString(ry));
	}
	else {
		if (angle1 < 0)
			angle1 = math::TWO_PI+angle1;
		if (angle2 < 0)
			angle2 = math::TWO_PI+angle2;
		elem = new XMLElementNode("path");
		int large_arg = fabs(angle1-angle2) > math::PI ? 0 : 1;
		int sweep_flag = angle1 > angle2 ? 0 : 1;
		if (angle1 > angle2) {
			large_arg = 1-large_arg;
			sweep_flag = 1-sweep_flag;
		}
		ostringstream oss;
		oss << 'M' << XMLString(x+rx*cos(angle1)) << ',' << XMLString(y+ry*sin(-angle1))
			 << 'A' << XMLString(rx) << ',' << XMLString(ry)
			 << " 0 "
			 << large_arg << ' ' << sweep_flag << ' '
			 << XMLString(x+rx*cos(angle2)) << ',' << XMLString(y-ry*sin(angle2));
		if (_fill >= 0)
			oss << 'Z';
		elem->addAttribute("d", oss.str());
	}
	elem->addAttribute("stroke-width", _penwidth);
	elem->addAttribute("stroke", actions.getColor().svgColorString());
	elem->addAttribute("stroke-linecap", "round");
	elem->addAttribute("fill", "none");
	if (_fill >= 0) {
		Color color=actions.getColor();
		color *= _fill;
		elem->addAttribute("fill", color.svgColorString());
	}
	else
		elem->addAttribute("fill", "none");
	actions.appendToPage(elem);
	actions.embed(BoundingBox(cx-rx, cy-ry, cx+rx, cy+ry));
	reset();
}


static constexpr int cmd_id (int a, int b) {
	return (a << 8) | b;
};

bool TpicSpecialHandler::process (const char *prefix, istream &is, SpecialActions &actions) {
	if (!prefix || strlen(prefix) != 2)
		return false;

	const double mi2bp=0.072; // factor for milli-inch to PS points
	StreamInputBuffer ib(is);
	BufferInputReader ir(ib);
	switch (cmd_id(prefix[0], prefix[1])) {
		case cmd_id('p','n'): // set pen width in milli-inches
			_penwidth = ir.getDouble()*mi2bp;
			break;
		case cmd_id('b','k'): // set fill color to black
			_fill = 0;
			break;
		case cmd_id('w','h'): // set fill color to white
			_fill = 1;
			break;
		case cmd_id('s','h'): // set fill color to given gray level
			ir.skipSpace();
			_fill = ir.eof() ? 0.5 : max(0.0, min(1.0, ir.getDouble()));
			break;
		case cmd_id('t','x'): // set fill pattern
			break;
		case cmd_id('p','a'): { // add point to path
			double x = ir.getDouble()*mi2bp;
			double y = ir.getDouble()*mi2bp;
			_points.emplace_back(DPair(x,y));
			break;
		}
		case cmd_id('f','p'): // draw solid lines through recorded points; close and fill path if fill color was defined
			drawLines(true, _fill >= 0, 0, actions);
			break;
		case cmd_id('i','p'): // don't draw outlines but close the recorded path and fill the resulting polygon
			drawLines(false, true, 0, actions);
			break;
		case cmd_id('d','a'): // as fp but draw dashed lines
			drawLines(true, _fill >= 0, ir.getDouble()*72, actions);
			break;
		case cmd_id('d','t'): // as fp but draw dotted lines
			drawLines(true, _fill >= 0, -ir.getDouble()*72, actions);
			break;
		case cmd_id('s','p'): { // draw quadratic splines through recorded points
			double ddist = ir.getDouble();
			drawSplines(ddist, actions);
			break;
		}
		case cmd_id('a','r'): { // draw elliptical arc
			double cx = ir.getDouble()*mi2bp;
			double cy = ir.getDouble()*mi2bp;
			double rx = ir.getDouble()*mi2bp;
			double ry = ir.getDouble()*mi2bp;
			double a1 = ir.getDouble();
			double a2 = ir.getDouble();
			drawArc(cx, cy, rx, ry, a1, a2, actions);
			break;
		}
		case cmd_id('i','a'): { // fill elliptical arc
			double cx = ir.getDouble()*mi2bp;
			double cy = ir.getDouble()*mi2bp;
			double rx = ir.getDouble()*mi2bp;
			double ry = ir.getDouble()*mi2bp;
			double a1 = ir.getDouble();
			double a2 = ir.getDouble();
			if (_fill < 0)
				_fill = 1;
			drawArc(cx, cy, rx, ry, a1, a2, actions);
			if (_fill < 0)
				_fill = -1;
			break;
		}
		default:
			return false;
	}
	return true;
}


const char** TpicSpecialHandler::prefixes () const {
	static const char *pfx[] = {"ar", "bk", "da", "dt", "fp", "ia", "ip", "pa", "pn", "sh", "sp", "tx", "wh", 0};
	return pfx;
}
