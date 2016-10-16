/*************************************************************************
** HtmlSpecialHandler.cpp                                               **
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
#include <cassert>
#include <sstream>
#include "HtmlSpecialHandler.hpp"
#include "InputReader.hpp"
#include "Message.hpp"
#include "SpecialActions.hpp"
#include "SVGTree.hpp"

using namespace std;

// variable to select the link marker variant (none, underlined, boxed, or colored background)
HtmlSpecialHandler::MarkerType HtmlSpecialHandler::MARKER_TYPE = HtmlSpecialHandler::MarkerType::LINE;
Color HtmlSpecialHandler::LINK_BGCOLOR;
Color HtmlSpecialHandler::LINK_LINECOLOR;
bool HtmlSpecialHandler::USE_LINECOLOR = false;


void HtmlSpecialHandler::preprocess (const char *, istream &is, SpecialActions &actions) {
	StreamInputReader ir(is);
	ir.skipSpace();
	// collect page number and ID of named anchors
	map<string,string> attribs;
	if (ir.check("<a ") && ir.parseAttributes(attribs, '"') > 0) {
		map<string,string>::iterator it;
		if ((it = attribs.find("name")) != attribs.end())
			preprocessNameAnchor(it->second, actions);
		else if ((it = attribs.find("href")) != attribs.end())
			preprocessHrefAnchor(it->second);
	}
}


void HtmlSpecialHandler::preprocessNameAnchor (const string &name, SpecialActions &actions) {
	NamedAnchors::iterator it = _namedAnchors.find(name);
	if (it == _namedAnchors.end()) {  // anchor completely undefined?
		int id = static_cast<int>(_namedAnchors.size())+1;
		_namedAnchors[name] = NamedAnchor(actions.getCurrentPageNumber(), id, 0);
	}
	else if (it->second.id < 0) {  // anchor referenced but not defined yet?
		it->second.id *= -1;
		it->second.pageno = actions.getCurrentPageNumber();
	}
	else
		Message::wstream(true) << "named hyperref anchor '" << name << "' redefined\n";
}


void HtmlSpecialHandler::preprocessHrefAnchor (const string &uri) {
	if (uri[0] != '#')
		return;
	string name = uri.substr(1);
	NamedAnchors::iterator it = _namedAnchors.find(name);
	if (it != _namedAnchors.end())  // anchor already defined?
		it->second.referenced = true;
	else {
		int id = static_cast<int>(_namedAnchors.size())+1;
		_namedAnchors[name] = NamedAnchor(0, -id, 0, true);
	}
}


bool HtmlSpecialHandler::process (const char *, istream &is, SpecialActions &actions) {
	_active = true;
	StreamInputReader ir(is);
	ir.skipSpace();
	map<string,string> attribs;
	map<string,string>::iterator it;
	if (ir.check("<a ") && ir.parseAttributes(attribs, '"') > 0) {
		if ((it = attribs.find("href")) != attribs.end())   // <a href="URI">
			processHrefAnchor(it->second, actions);
		else if ((it = attribs.find("name")) != attribs.end())  // <a name="ID">
			processNameAnchor(it->second, actions);
		else
			return false;  // none or only invalid attributes
	}
	else if (ir.check("</a>"))
		closeAnchor(actions);
	else if (ir.check("<img src=")) {
	}
	else if (ir.check("<base ") && ir.parseAttributes(attribs, '"') > 0 && (it = attribs.find("href")) != attribs.end())
		_base = it->second;
	return true;
}


/** Handles anchors with href attribute: <a href="URI">...</a>
 *  @param uri value of href attribute */
void HtmlSpecialHandler::processHrefAnchor (string uri, SpecialActions &actions) {
	closeAnchor(actions);
	string name;
	if (uri[0] == '#') {  // reference to named anchor?
		name = uri.substr(1);
		NamedAnchors::iterator it = _namedAnchors.find(name);
		if (it == _namedAnchors.end() || it->second.id < 0)
			Message::wstream(true) << "reference to undefined anchor '" << name << "'\n";
		else {
			int id = it->second.id;
			uri = "#loc"+XMLString(id);
			if (actions.getCurrentPageNumber() != it->second.pageno) {
				ostringstream oss;
				oss << actions.getSVGFilename(it->second.pageno) << uri;
				uri = oss.str();
			}
		}
	}
	if (!_base.empty() && uri.find("://") != string::npos) {
		if (*_base.rbegin() != '/' && uri[0] != '/' && uri[0] != '#')
			uri = "/" + uri;
		uri = _base + uri;
	}
	XMLElementNode *anchor = new XMLElementNode("a");
	anchor->addAttribute("xlink:href", uri);
	anchor->addAttribute("xlink:title", XMLString(name.empty() ? uri : name, false));
	actions.pushContextElement(anchor);
	actions.bbox("{anchor}", true);  // start computing the bounding box of the linked area
	_depthThreshold = actions.getDVIStackDepth();
	_anchorType = AnchorType::HREF;
}


/** Handles anchors with name attribute: <a name="NAME">...</a>
 *  @param name value of name attribute */
void HtmlSpecialHandler::processNameAnchor (const string &name, SpecialActions &actions) {
	closeAnchor(actions);
	NamedAnchors::iterator it = _namedAnchors.find(name);
	assert(it != _namedAnchors.end());
	it->second.pos = actions.getY();
	_anchorType = AnchorType::NAME;
}


/** Handles the closing tag (</a> of the current anchor element. */
void HtmlSpecialHandler::closeAnchor (SpecialActions &actions) {
	if (_anchorType == AnchorType::HREF) {
		markLinkedBox(actions);
		actions.popContextElement();
		_depthThreshold = 0;
	}
	_anchorType = AnchorType::NONE;
}


/** Marks a single rectangular area of the linked part of the document with a line or
 *  a box so that it's noticeable by the user. Additionally, an invisible rectangle is
 *  placed over this area in order to avoid flickering of the mouse cursor when moving
 *  it over the hyperlinked area. */
void HtmlSpecialHandler::markLinkedBox (SpecialActions &actions) {
	const BoundingBox &bbox = actions.bbox("{anchor}");
	if (bbox.width() > 0 && bbox.height() > 0) {  // does the bounding box extend in both dimensions?
		if (MARKER_TYPE != MarkerType::NONE) {
			const double linewidth = min(0.5, bbox.height()/15);
			XMLElementNode *rect = new XMLElementNode("rect");
			double x = bbox.minX();
			double y = bbox.maxY()+linewidth;
			double w = bbox.width();
			double h = linewidth;
			const Color &linecolor = USE_LINECOLOR ? LINK_LINECOLOR : actions.getColor();
			if (MARKER_TYPE == MarkerType::LINE)
				rect->addAttribute("fill", linecolor.svgColorString());
			else {
				x -= linewidth;
				y = bbox.minY()-linewidth;
				w += 2*linewidth;
				h += bbox.height()+linewidth;
				if (MARKER_TYPE == MarkerType::BGCOLOR) {
					rect->addAttribute("fill", LINK_BGCOLOR.svgColorString());
					if (USE_LINECOLOR) {
						rect->addAttribute("stroke", linecolor.svgColorString());
						rect->addAttribute("stroke-width", linewidth);
					}
				}
				else {  // LM_BOX
					rect->addAttribute("fill", "none");
					rect->addAttribute("stroke", linecolor.svgColorString());
					rect->addAttribute("stroke-width", linewidth);
				}
			}
			rect->addAttribute("x", x);
			rect->addAttribute("y", y);
			rect->addAttribute("width", w);
			rect->addAttribute("height", h);
			actions.prependToPage(rect);
			if (MARKER_TYPE == MarkerType::BOX || MARKER_TYPE == MarkerType::BGCOLOR) {
				// slightly enlarge the boxed area
				x -= linewidth;
				y -= linewidth;
				w += 2*linewidth;
				h += 2*linewidth;
			}
			actions.embed(BoundingBox(x, y, x+w, y+h));
		}
		// Create an invisible rectangle around the linked area so that it's easier to access.
		// This is only necessary when using paths rather than real text elements together with fonts.
		if (!SVGTree::USE_FONTS) {
			XMLElementNode *rect = new XMLElementNode("rect");
			rect->addAttribute("x", bbox.minX());
			rect->addAttribute("y", bbox.minY());
			rect->addAttribute("width", bbox.width());
			rect->addAttribute("height", bbox.height());
			rect->addAttribute("fill", "white");
			rect->addAttribute("fill-opacity", 0);
			actions.appendToPage(rect);
		}
	}
}


/** This method is called every time the DVI position changes. */
void HtmlSpecialHandler::dviMovedTo (double x, double y, SpecialActions &actions) {
	if (_active && _anchorType != AnchorType::NONE) {
		// Start a new box if the current depth of the DVI stack underruns
		// the initial threshold which indicates a line break.
		if (actions.getDVIStackDepth() < _depthThreshold) {
			markLinkedBox(actions);
			_depthThreshold = actions.getDVIStackDepth();
			actions.bbox("{anchor}", true);  // start a new box on the new line
		}
	}
}


void HtmlSpecialHandler::dviEndPage (unsigned pageno, SpecialActions &actions) {
	if (_active) {
		// create views for all collected named anchors defined on the recent page
		const BoundingBox &pagebox = actions.bbox();
		for (auto &stranchorpair : _namedAnchors) {
			if (stranchorpair.second.pageno == pageno && stranchorpair.second.referenced) {  // current anchor referenced?
				ostringstream oss;
				oss << pagebox.minX() << ' ' << stranchorpair.second.pos << ' '
					 << pagebox.width() << ' ' << pagebox.height();
				XMLElementNode *view = new XMLElementNode("view");
				view->addAttribute("id", "loc"+XMLString(stranchorpair.second.id));
				view->addAttribute("viewBox", oss.str());
				actions.appendToDefs(view);
			}
		}
		closeAnchor(actions);
		_active = false;
	}
}


/** Sets the appearance of the link markers.
 *  @param[in] marker string specifying the marker (format: type[:linecolor])
 *  @return true on success */
bool HtmlSpecialHandler::setLinkMarker (const string &marker) {
	string type;  // "none", "box", "line", or a background color specifier
	string color; // optional line color specifier
	size_t seppos = marker.find(":");
	if (seppos == string::npos)
		type = marker;
	else {
		type = marker.substr(0, seppos);
		color = marker.substr(seppos+1);
	}
	if (type.empty() || type == "none")
		MARKER_TYPE = MarkerType::NONE;
	else if (type == "line")
		MARKER_TYPE = MarkerType::LINE;
	else if (type == "box")
		MARKER_TYPE = MarkerType::BOX;
	else {
		if (!LINK_BGCOLOR.setPSName(type, false))
			return false;
		MARKER_TYPE = MarkerType::BGCOLOR;
	}
	USE_LINECOLOR = false;
	if (MARKER_TYPE != MarkerType::NONE && !color.empty()) {
		if (!LINK_LINECOLOR.setPSName(color, false))
			return false;
		USE_LINECOLOR = true;
	}
	return true;
}


const char** HtmlSpecialHandler::prefixes () const {
	static const char *pfx[] = {"html:", 0};
	return pfx;
}
