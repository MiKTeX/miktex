/*************************************************************************
** PSPattern.cpp                                                        **
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

#include <array>
#include <sstream>
#include <vector>
#include "BoundingBox.hpp"
#include "PSPattern.hpp"
#include "SpecialActions.hpp"
#include "SVGTree.hpp"
#include "utility.hpp"
#include "XMLNode.hpp"

using namespace std;


string PSPattern::svgID () const {
	return XMLString("pat")+XMLString(_id);
}


/** Appends the definition of this pattern to the "def" section of the SVG tree. */
void PSPattern::apply (SpecialActions &actions) {
	if (auto pattern = createPatternNode())
		actions.svgTree().appendToDefs(std::move(pattern));
}


/////////////////////////////////////////////////////////////////////////////

PSTilingPattern::PSTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep)
	: PSPattern(id), _bbox(bbox), _matrix(matrix), _xstep(xstep), _ystep(ystep)
{
	_groupNode = PSTilingPattern::createGroupNode();
	_groupNodePtr = _groupNode.get();
}


/** Assigns a new group element. */
void PSTilingPattern::setGroupNode (unique_ptr<XMLElement> node) {
	_groupNode = std::move(node);
	_groupNodePtr = _groupNode.get();
}


/** Creates a new pattern element representing the pattern defined in the PS code. */
unique_ptr<XMLElement> PSTilingPattern::createPatternNode () const {
	if (!_groupNode)
		return nullptr;
	BoundingBox box(_bbox.minX(), _bbox.minY(), _bbox.minX()+_xstep, _bbox.minY()+_ystep);
	auto pattern = util::make_unique<XMLElement>("pattern");
	pattern->addAttribute("id", svgID());
	pattern->addAttribute("x", box.minX());
	pattern->addAttribute("y", box.minY());
	pattern->addAttribute("width", box.width());
	pattern->addAttribute("height", box.height());
	pattern->addAttribute("viewBox", box.toSVGViewBox());
	pattern->addAttribute("patternUnits", "userSpaceOnUse");
	if (!_matrix.isIdentity())
		pattern->addAttribute("patternTransform", _matrix.toSVG());
	if (_xstep < _bbox.width() || _ystep < _bbox.height()) {  // overlapping tiles?
		// disable clipping at the tile borders => tiles become "transparent"
		pattern->addAttribute("overflow", "visible");
	}
	if (auto clip = createClipNode())
		pattern->append(std::move(clip));
	pattern->append(std::move(_groupNode));
	_groupNode.reset();
	return pattern;
}


/** Creates a new clip element restricting the drawing area to the
 *  dimensions given in the definition of the pattern. */
unique_ptr<XMLElement> PSTilingPattern::createClipNode() const {
	auto clip = util::make_unique<XMLElement>("clipPath");
	clip->addAttribute("id", "pc"+XMLString(psID()));
	auto rect = util::make_unique<XMLElement>("rect");
	rect->addAttribute("x", _bbox.minX());
	rect->addAttribute("y", _bbox.minY());
	rect->addAttribute("width", _bbox.width());
	rect->addAttribute("height", _bbox.height());
	clip->append(std::move(rect));
	return clip;
}


/** Creates a new group element that contains all "drawing" elements that
 *  define the pattern graphic. */
unique_ptr<XMLElement> PSTilingPattern::createGroupNode () const {
	// add all succeeding path elements to this group
	auto group = util::make_unique<XMLElement>("g");
	group->addAttribute("clip-path", XMLString("url(#pc")+XMLString(psID())+")");
	return group;
}


void PSTilingPattern::apply (SpecialActions &actions) {
	PSPattern::apply(actions);
	_groupNode.reset();
}


/////////////////////////////////////////////////////////////////////////////

PSColoredTilingPattern::PSColoredTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep)
	: PSTilingPattern(id, bbox, matrix, xstep, ystep)
{
}


/////////////////////////////////////////////////////////////////////////////

PSUncoloredTilingPattern::PSUncoloredTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep)
	: PSTilingPattern(id, bbox, matrix, xstep, ystep), _applied()
{
}


/** Returns an SVG id value that identifies this pattern with the current color applied. */
string PSUncoloredTilingPattern::svgID () const {
	ostringstream oss;
	oss << PSPattern::svgID() << '-' << hex << uint32_t(_currentColor);
	return oss.str();
}


/** Appends the definition of this pattern with the current color applied
 *  to the "def" section of the SVG tree. */
void PSUncoloredTilingPattern::apply (SpecialActions &actions) {
	auto it=_colors.find(_currentColor);
	if (it == _colors.end()) {
		if (_applied)
			setGroupNode(util::static_unique_ptr_cast<XMLElement>(getGroupNode()->clone()));
		// assign current color to the pattern graphic
		vector<XMLElement*> colored_elems;
		for (const char *attrName : {"fill", "stroke"}) {
			getGroupNode()->getDescendants(nullptr, attrName, colored_elems);
			for (XMLElement *elem : colored_elems) {
				if (string(elem->getAttributeValue(attrName)) != "none")
					elem->addAttribute(attrName, _currentColor.svgColorString());
			}
			colored_elems.clear();
		}
		PSPattern::apply(actions);
		_colors.insert(_currentColor);
		_applied = true;
	}
}


unique_ptr<XMLElement> PSUncoloredTilingPattern::createClipNode() const {
	// only the first instance of this patterns get a clip element
	if (_colors.empty())
		return PSTilingPattern::createClipNode();
	return nullptr;
}
