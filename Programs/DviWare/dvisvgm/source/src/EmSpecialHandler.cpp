/*************************************************************************
** EmSpecialHandler.cpp                                                 **
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

#include <sstream>
#include "EmSpecialHandler.hpp"
#include "InputBuffer.hpp"
#include "InputReader.hpp"
#include "Length.hpp"
#include "SpecialActions.hpp"
#include "SVGTree.hpp"
#include "XMLNode.hpp"
#include "XMLString.hpp"

using namespace std;


/** Computes the "cut vector" that is used to compute the line shape.
 *  Because each line has a width > 0 the actual shape of the line is a tetragon.
 *  The 4 vertices can be influenced by the cut parameter c that specifies
 *  a horizontal, vertical or orthogonal cut of a line end. Depending on c and the
 *  line's slope a cut vector v can be computed that, relatively to endpoint p, denotes
 *  the 2 vertices of that line end: v1=p+v and v2=p-v.
 *  @param[in] cuttype character identifying the cut direction ('h', 'v' or 'p')
 *  @param[in] linedir direction vector of line to be drawn
 *  @param[in] lw width of line to be drawn
 *  @return the "cut vector" */
static DPair cut_vector (char cuttype, const DPair &linedir, double linewidth) {
	DPair cut;
	switch (cuttype) {
		case 'v':  // vertical
			if (linedir.x() != 0) {
				double slope = linedir.y()/linedir.x();
				double h = sqrt(linewidth*linewidth*(1+slope*slope));
				cut.y(h/2);
			}
			break;
		case 'h':  // horizontal
			if (linedir.y() != 0) {
				double slope = linedir.x()/linedir.y();
				double h = sqrt(linewidth*linewidth*(1+slope*slope));
				double sgn = slope < 0 ? 1.0 : -1.0;
				cut.x(h*sgn/2);
			}
			break;
		default: // c == 'p': perpendicular to the line vector
			if (linedir.x() != 0 && linedir.y() != 0)
				return linedir.ortho()/linedir.length() * (linewidth/2);
	}
	return cut;
}


/** Creates the SVG element that will a the line.
 * @param[in] p1 first endpoint in PS point units
 * @param[in] p2 second endpoint in PS point units
 * @param[in] c1 cut method of first endpoint ('h', 'v' or 'p')
 * @param[in] c2 cut method of second endpoint ('h', 'v' or 'p')
 * @param[in] lw line width in PS point units
 * @param[in] actions object providing the actions that can be performed by the SpecialHandler */
static void create_line (const DPair &p1, const DPair &p2, char c1, char c2, double lw, SpecialActions &actions) {
	if (actions.outputLocked())
		return;
	unique_ptr<XMLElement> node;
	DPair dir = p2-p1;
	if (dir.x() == 0 || dir.y() == 0 || (c1 == 'p' && c2 == 'p')) {
		// draw regular line
		node = util::make_unique<XMLElement>("line");
		node->addAttribute("x1", p1.x());
		node->addAttribute("y1", p1.y());
		node->addAttribute("x2", p2.x());
		node->addAttribute("y2", p2.y());
		node->addAttribute("stroke-width", lw);
		node->addAttribute("stroke", actions.getColor().svgColorString());
		// update bounding box
		DPair cv = cut_vector('p', dir, lw);
		actions.embed(p1+cv);
		actions.embed(p1-cv);
		actions.embed(p2+cv);
		actions.embed(p2-cv);
	}
	else {
		// draw polygon
		DPair cv1 = cut_vector(c1, dir, lw);
		DPair cv2 = cut_vector(c2, dir, lw);
		DPair q11 = p1+cv1, q12 = p1-cv1;
		DPair q21 = p2+cv2, q22 = p2-cv2;
		ostringstream oss;
		oss << XMLString(q11.x()) << ',' << XMLString(q11.y()) << ' '
			 << XMLString(q12.x()) << ',' << XMLString(q12.y()) << ' '
			 << XMLString(q22.x()) << ',' << XMLString(q22.y()) << ' '
			 << XMLString(q21.x()) << ',' << XMLString(q21.y());
		node = util::make_unique<XMLElement>("polygon");
		node->addAttribute("points", oss.str());
		if (actions.getColor() != Color::BLACK)
			node->addAttribute("fill", actions.getColor().svgColorString());
		// update bounding box
		actions.embed(q11);
		actions.embed(q12);
		actions.embed(q21);
		actions.embed(q22);
	}
	actions.svgTree().appendToPage(std::move(node));
}


/** Reads a length (value + unit) and returns its value in PS points (bp).
 *  If no unit is specified, TeX points are assumed. */
static double read_length (InputReader &in) {
	double val = in.getDouble();
	string unitstr;
	if (isalpha(in.peek())) unitstr += char(in.get());
	if (isalpha(in.peek())) unitstr += char(in.get());
	Length::Unit unit = Length::Unit::PT;
	try {
		unit = Length::stringToUnit(unitstr);
	}
	catch (UnitException &e) {
	}
	return Length(val, unit).bp();
}


bool EmSpecialHandler::process (const string &prefix, istream &is, SpecialActions &actions) {
	// em:moveto => move graphic cursor to dvi position
	// em:lineto => draw line from graphic cursor to dvi cursor, then move graphic cursor to dvi position
	// em:linewidth <w> => set line width to <w>
	// em:point <n>[,<x>[,<y>]] => defines point <n> as (<x>,<y>); if <x> and/or <y> is missing,
	//                             the corresponding dvi cursor coordinate is inserted
	//                             <x> and <y> are lengths
	// em:line <n>[h|v|p], <m>[h|v|p] [,<w>] => draws line of width <w> from point #<n> to point #<m>
	// 	point number suffixes:
	// 	  h: cut line horizontally
	// 	  v: cut line vertically
	// 	  p: cut line orthogonally to line direction (default)
	//    if <w> is omitted, the global line width is used
	//
	// supported length units: pt, pc, in, bp, cm, mm, dd, cc, sp
	// default line width: 0.4pt
	struct Command {
		const char *name;
		void (EmSpecialHandler::*handler)(InputReader&, SpecialActions&);
	} commands[] = {
		{"point",     &EmSpecialHandler::point},
		{"line",      &EmSpecialHandler::line},
		{"moveto",    &EmSpecialHandler::moveto},
		{"lineto",    &EmSpecialHandler::lineto},
		{"linewidth", &EmSpecialHandler::linewidth},
		{nullptr, nullptr}
	};

	StreamInputReader ir(is);
	const string cmdstr = ir.getWord();
	for (Command *cmd=commands; cmd->name; cmd++) {
		if (cmdstr == cmd->name) {
			(this->*cmd->handler)(ir, actions);
			break;
		}
	}
	return true;
}


/** Handles the "moveto" command that sets the drawing position to the current DVI position. */
void EmSpecialHandler::moveto (InputReader&, SpecialActions &actions) {
	_pos.x(actions.getX());
	_pos.y(actions.getY());
}


/** Handles the "lineto" command that sraws a straight line from the current drawing position
 *  to the current DVI position, and sets the drawing position to the DVI position afterwards. */
void EmSpecialHandler::lineto (InputReader&, SpecialActions &actions) {
	DPair currpos(actions.getX(), actions.getY());
	create_line(_pos, currpos, 'p', 'p', _linewidth, actions);
	_pos = currpos;
}


/** Handles the "linewidth" command that changes the line width which affects all following
 *  drawing commands. */
void EmSpecialHandler::linewidth (InputReader &ir, SpecialActions&) {
	_linewidth = read_length(ir);
}


/** Handles the "point" command that adds a point to the point list. */
void EmSpecialHandler::point (InputReader &ir, SpecialActions &actions) {
	DPair pos(actions.getX(), actions.getY());
	int n = ir.getInt();
	if (ir.getPunct() == ',') {
		pos.x(ir.getDouble());
		if (ir.getPunct() == ',')
			pos.y(ir.getDouble());
	}
	_points[n] = pos;
}


/** Handles the "line" command that draws a straight line between two points
 *  from the point list. */
void EmSpecialHandler::line (InputReader &ir, SpecialActions& actions) {
	int pointnum1 = ir.getInt();
	int cut1 = 'p';
	if (isalpha(ir.peek()))
		cut1 = ir.get();
	ir.getPunct();
	int pointnum2 = ir.getInt();
	int cut2 = 'p';
	if (isalpha(ir.peek()))
		cut2 = ir.get();
	double linewidth = _linewidth;
	if (ir.getPunct() == ',')
		linewidth = read_length(ir);
	auto it1=_points.find(pointnum1);
	auto it2=_points.find(pointnum2);
	if (it1 != _points.end() && it2 != _points.end())
		create_line(it1->second, it2->second, char(cut1), char(cut2), linewidth, actions);
	else {
		// Line endpoints don't necessarily have to be defined before
		// a line definition. If a point isn't defined yet, we put the line
		// in a wait list and process the lines at the end of the page.
		_lines.emplace_back(Line(pointnum1, pointnum2, char(cut1), char(cut2), linewidth));
	}
}


/** This method is called at the end of a DVI page. Here we have to draw all pending
 *   lines that are still in the line list. All line endpoints must be defined until here. */
void EmSpecialHandler::dviEndPage (unsigned pageno, SpecialActions &actions) {
	for (const Line &line : _lines) {
		auto pit1=_points.find(line.p1);
		auto pit2=_points.find(line.p2);
		if (pit1 != _points.end() && pit2 != _points.end())
			create_line(pit1->second, pit2->second, line.c1, line.c2, line.width, actions);
		// all lines with still undefined points are ignored
	}
	// line and point definitions are local to a page
	_lines.clear();
	_points.clear();
}


vector<const char*> EmSpecialHandler::prefixes() const {
	vector<const char*> pfx {"em:"};
	return pfx;
}
