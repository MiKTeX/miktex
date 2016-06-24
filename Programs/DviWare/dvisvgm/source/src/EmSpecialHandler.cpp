/*************************************************************************
** EmSpecialHandler.cpp                                                 **
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

#include <config.h>
#include <sstream>
#include "EmSpecialHandler.h"
#include "InputBuffer.h"
#include "InputReader.h"
#include "SpecialActions.h"
#include "XMLNode.h"
#include "XMLString.h"

using namespace std;


EmSpecialHandler::EmSpecialHandler () : _linewidth(0.4*72/72.27), _actions(0) {
}


/** Computes the "cut vector" that is used to compute the line shape.
 *  Because each line has a width > 0 the actual shape of the line is a tetragon.
 *  The 4 vertices can be influenced by the cut parameter c that specifies
 *  a horizontal, vertical or orthogonal cut of a line end. Depending on c and the
 *  line's slope a cut vector v can be computed that, relatively to endpoint p, denotes
 *  the 2 vertices of that line end: v1=p+v and v2=p-v.
 *  @param[in] c cut direction ('h', 'v' or 'p')
 *  @param[in] v direction vector of line to be drawn
 *  @param[in] lw width of line to be drawn
 *  @return the "cut vector" */
static DPair cut_vector (char c, const DPair &v, double lw) {
	if (c == 'p')
		return v.ortho()/v.length() * (lw/2);
	DPair cut;
	if (c == 'v' && v.x() != 0) {
		double slope = v.y()/v.x();
		double h = sqrt(lw*lw*(1+slope*slope));
		cut.y(0.5*h);
	}
	else if (v.y() != 0) {  // c == 'h'
		double slope = v.x()/v.y();
		double h = sqrt(lw*lw*(1+slope*slope));
		double sgn = slope < 0 ? 1.0 : -1.0;
		cut.x(0.5*h*sgn);
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
static void create_line (const DPair &p1, const DPair &p2, char c1, char c2, double lw, SpecialActions *actions) {
	XMLElementNode *node=0;
	DPair dir = p2-p1;
	if (dir.x() == 0 || dir.y() == 0 || (c1 == 'p' && c2 == 'p')) {
		// draw regular line
		node = new XMLElementNode("line");
		node->addAttribute("x1", p1.x());
		node->addAttribute("y1", p1.y());
		node->addAttribute("x2", p2.x());
		node->addAttribute("y2", p2.y());
		node->addAttribute("stroke-width", lw);
		node->addAttribute("stroke", actions->getColor().svgColorString());
		// update bounding box
		actions->embed(p1);
		actions->embed(p2);
	}
	else {
		// draw polygon
		DPair cv1 = cut_vector(c1, dir, lw);
		DPair cv2 = cut_vector(c2, dir, lw);
		DPair q11 = p1+cv1, q12 = p1-cv1;
		DPair q21 = p2+cv2, q22 = p2-cv2;
		node = new XMLElementNode("polygon");
		ostringstream oss;
		oss << XMLString(q11.x()) << ',' << XMLString(q11.y()) << ' '
			 << XMLString(q12.x()) << ',' << XMLString(q12.y()) << ' '
			 << XMLString(q22.x()) << ',' << XMLString(q22.y()) << ' '
			 << XMLString(q21.x()) << ',' << XMLString(q21.y());
		node->addAttribute("points", oss.str());
		if (actions->getColor() != Color::BLACK)
			node->addAttribute("fill", actions->getColor().svgColorString());
		// update bounding box
		actions->embed(q11);
		actions->embed(q12);
		actions->embed(q21);
		actions->embed(q22);
	}
	actions->appendToPage(node);
}


static double read_length (InputReader &in) {
	struct Unit {
		const char *name;
		double factor;
	}
	units[] = {
		{"pt", 1.0},
		{"pc", 12.0},
		{"in", 72.27},
		{"bp", 72.27/72},
		{"cm", 72.27/2.54},
		{"mm", 72.27/25.4},
		{"dd", 1238.0/1157},
		{"cc", 1238.0/1157*12},
		{"sp", 1.0/65536},
		{0, 0}
	};
	double len = in.getDouble();
	in.skipSpace();
	for (Unit *p=units; p->name; p++)
		if (in.check(p->name)) {
			len *= p->factor;
			break;
		}
	return len*72/72.27;
}


bool EmSpecialHandler::process (const char *prefix, istream &is, SpecialActions *actions) {
	// em:moveto => move graphic cursor to dvi cursor
	// em:lineto => draw line from graphic cursor to dvi cursor, move graphic cursor to dvi cursor
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

	if (actions) {
		_actions = actions;  // save pointer to SpecialActions for later use in endPage
		StreamInputBuffer ib(is, 128);
		BufferInputReader in(ib);
		string cmd = in.getWord();
		if (cmd == "moveto")
			_pos = DPair(actions->getX(), actions->getY());
		else if (cmd == "lineto") {
			DPair p(actions->getX(), actions->getY());
			create_line(_pos, p, 'p', 'p', _linewidth, actions);
			_pos = p;
		}
		else if (cmd == "linewidth")
			_linewidth = read_length(in);
		else if (cmd == "point") {
			DPair p(actions->getX(), actions->getY());
			int n = in.getInt();
			if (in.getPunct() == ',') {
				p.x(in.getDouble());
				if (in.getPunct() == ',')
					p.y(in.getDouble());
			}
			_points[n] = p;
		}
		else if (cmd == "line") {
			int n1 = in.getInt();
			int c1 = 'p';
			if (isalpha(in.peek()))
				c1 = in.get();
			in.getPunct();
			int n2 = in.getInt();
			int c2 = 'p';
			if (isalpha(in.peek()))
				c2 = in.get();
			double lw = _linewidth;
			if (in.getPunct() == ',')
				lw = read_length(in);
			map<int,DPair>::iterator it1=_points.find(n1);
			map<int,DPair>::iterator it2=_points.find(n2);
			if (it1 != _points.end() && it2 != _points.end())
				create_line(it1->second, it2->second, char(c1), char(c2), lw, actions);
			else {
				// Line endpoints havn't necessarily to be defined before
				// a line definition. If a point wasn't defined yet we push the line
				// in a wait list and process the lines at the end of the page.
				_lines.push_back(Line(n1, n2, char(c1), char(c2), lw));
			}
		}
	}
	return true;
}


/** This method is called at the end of a DVI page. Here we have to draw all pending
 *   lines that are still in the line list. All line endpoints must be defined until here. */
void EmSpecialHandler::dviEndPage (unsigned pageno) {
	if (_actions) {
		FORALL(_lines, list<Line>::iterator, it) {
			map<int,DPair>::iterator pit1=_points.find(it->p1);
			map<int,DPair>::iterator pit2=_points.find(it->p2);
			if (pit1 != _points.end() && pit2 != _points.end())
				create_line(pit1->second, pit2->second, it->c1, it->c2, it->width, _actions);
			// all lines with still undefined points are ignored
		}
	}
	// line and point definitions are local to a page
	_lines.clear();
	_points.clear();
}


const char** EmSpecialHandler::prefixes () const {
	static const char *pfx[] = {"em:", 0};
	return pfx;
}
