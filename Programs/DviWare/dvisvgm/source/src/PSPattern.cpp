/*************************************************************************
** PSPattern.cpp                                                        **
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
#include <vector>
#include "BoundingBox.h"
#include "PSPattern.h"
#include "SpecialActions.h"
#include "SVGTree.h"
#include "XMLNode.h"

using namespace std;


string PSPattern::svgID () const {
	return XMLString("pat")+XMLString(_id);
}


/** Appends the definition of this pattern to the "def" section of the SVG tree. */
void PSPattern::apply (SpecialActions *actions) {
	if (XMLElementNode *pattern = createPatternNode())
		actions->appendToDefs(pattern);
}


/////////////////////////////////////////////////////////////////////////////

PSTilingPattern::PSTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep)
	: PSPattern(id), _bbox(bbox), _matrix(matrix), _xstep(xstep), _ystep(ystep), _groupNode(0)
{
	_groupNode = PSTilingPattern::createGroupNode();
}


PSTilingPattern::~PSTilingPattern () {
	delete _groupNode;
}


/** Creates a new pattern element representing the pattern defined in the PS code. */
XMLElementNode* PSTilingPattern::createPatternNode () const {
	if (!_groupNode)
		return 0;
	BoundingBox box(_bbox.minX(), _bbox.minY(), _bbox.minX()+_xstep, _bbox.minY()+_ystep);
	XMLElementNode *pattern = new XMLElementNode("pattern");
	pattern->addAttribute("id", svgID());
	pattern->addAttribute("x", box.minX());
	pattern->addAttribute("y", box.minY());
	pattern->addAttribute("width", box.width());
	pattern->addAttribute("height", box.height());
	pattern->addAttribute("viewBox", box.toSVGViewBox());
	pattern->addAttribute("patternUnits", "userSpaceOnUse");
	if (!_matrix.isIdentity())
		pattern->addAttribute("patternTransform", _matrix.getSVG());
	if (_xstep < _bbox.width() || _ystep < _bbox.height()) {  // overlapping tiles?
		// disable clipping at the tile borders => tiles become "transparent"
		pattern->addAttribute("overflow", "visible");
	}
	if (XMLElementNode *clip=createClipNode())
		pattern->append(clip);
	pattern->append(_groupNode);
	return pattern;
}


/** Creates a new clip element restricting the drawing area to the
 *  dimensions given in the definition of the pattern. */
XMLElementNode* PSTilingPattern::createClipNode() const {
	XMLElementNode *clip = new XMLElementNode("clipPath");
	clip->addAttribute("id", "pc"+XMLString(psID()));
	XMLElementNode *rect = new XMLElementNode("rect");
	rect->addAttribute("x", _bbox.minX());
	rect->addAttribute("y", _bbox.minY());
	rect->addAttribute("width", _bbox.width());
	rect->addAttribute("height", _bbox.height());
	clip->append(rect);
	return clip;
}


/** Creates a new group element that contains all "drawing" elements that
 *  define the pattern graphic. */
XMLElementNode* PSTilingPattern::createGroupNode () const {
	// add all succeeding path elements to this group
	XMLElementNode *group = new XMLElementNode("g");
	group->addAttribute("clip-path", XMLString("url(#pc")+XMLString(psID())+")");
	return group;
}


void PSTilingPattern::apply (SpecialActions *actions) {
	PSPattern::apply(actions);
	_groupNode = 0;
}


/////////////////////////////////////////////////////////////////////////////

PSColoredTilingPattern::PSColoredTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep)
	: PSTilingPattern(id, bbox, matrix, xstep, ystep)
{
}


/////////////////////////////////////////////////////////////////////////////

PSUncoloredTilingPattern::PSUncoloredTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep)
	: PSTilingPattern(id, bbox, matrix, xstep, ystep), _applied(false)
{
}


PSUncoloredTilingPattern::~PSUncoloredTilingPattern () {
	if (_applied)
		setGroupNode(0);  // prevent deleting the group node in the parent destructor
}


/** Returns an SVG id value that identifies this pattern with the current color applied. */
string PSUncoloredTilingPattern::svgID () const {
	ostringstream oss;
	oss << PSPattern::svgID() << '-' << hex << _currentColor;
	return oss.str();
}


/** Appends the definition of this pattern with the current color applied
 *  to the "def" section of the SVG tree. */
void PSUncoloredTilingPattern::apply (SpecialActions* actions) {
	set<Color>::iterator it=_colors.find(_currentColor);
	if (it == _colors.end()) {
		if (_applied)
			setGroupNode(static_cast<XMLElementNode*>(getGroupNode()->clone()));
		// assign current color to the pattern graphic
		vector<XMLElementNode*> colored_elems;
		const char *attribs[] = {"fill", "stroke"};
		for (int i=0; i < 2; i++) {
			getGroupNode()->getDescendants(0, attribs[i], colored_elems);
			for (vector<XMLElementNode*>::iterator it=colored_elems.begin(); it != colored_elems.end(); ++it)
				if (string((*it)->getAttributeValue(attribs[i])) != "none")
					(*it)->addAttribute(attribs[i], _currentColor.svgColorString());
			colored_elems.clear();
		}
		PSPattern::apply(actions);
		_colors.insert(_currentColor);
		_applied = true;
	}
}


XMLElementNode* PSUncoloredTilingPattern::createClipNode() const {
	// only the first instance of this patterns get a clip element
	if (_colors.empty())
		return PSTilingPattern::createClipNode();
	return 0;
}
