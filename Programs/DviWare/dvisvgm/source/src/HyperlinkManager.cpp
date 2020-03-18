/*************************************************************************
** HyperlinkManager.cpp                                                 **
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
#include "HyperlinkManager.hpp"
#include "Message.hpp"
#include "SpecialActions.hpp"
#include "SVGTree.hpp"
#include "utility.hpp"
#include "XMLNode.hpp"

using namespace std;

// variable to select the link marker variant (none, underlined, boxed, or colored background)
HyperlinkManager::MarkerType HyperlinkManager::MARKER_TYPE = MarkerType::LINE;
Color HyperlinkManager::LINK_BGCOLOR;
Color HyperlinkManager::LINK_LINECOLOR;
HyperlinkManager::ColorSource HyperlinkManager::COLORSOURCE = ColorSource::DEFAULT;


HyperlinkManager& HyperlinkManager::instance () {
	static HyperlinkManager instance;
	return instance;
}


void HyperlinkManager::addHrefAnchor (const string &uri) {
	if (uri.empty() || uri[0] != '#')
		return;
	string name = uri.substr(1);
	auto it = _namedAnchors.find(name);
	if (it != _namedAnchors.end())  // anchor already defined?
		it->second.referenced = true;
	else {
		int id = static_cast<int>(_namedAnchors.size())+1;
		_namedAnchors.emplace(name, NamedAnchor(0, -id, 0, true));
	}
}


void HyperlinkManager::addNameAchor (const string &name, int pageno) {
	if (name.empty())
		return;
	auto it = _namedAnchors.find(name);
	if (it == _namedAnchors.end()) {  // anchor completely undefined?
		int id = static_cast<int>(_namedAnchors.size())+1;
		_namedAnchors.emplace(name, NamedAnchor(pageno, id, 0));
	}
	else if (it->second.id >= 0) // anchor already defined?
		Message::wstream(true) << "named hyperref anchor '" << name << "' redefined\n";
	else {
		// anchor referenced but not defined yet?
		it->second.id *= -1;
		it->second.pageno = pageno;
	}
}


void HyperlinkManager::setActiveNameAnchor (const string &name, SpecialActions &actions) {
	auto it = _namedAnchors.find(name);
	if (it != _namedAnchors.end()) {
		closeAnchor(actions);
		it->second.pos = actions.getY();
		_anchorType = AnchorType::NAME;
	}
}


void HyperlinkManager::createLink (string uri, SpecialActions &actions) {
	closeAnchor(actions);
	string name;
	if (uri[0] == '#') {  // reference to named anchor?
		name = uri.substr(1);
		auto it = _namedAnchors.find(name);
		if (it == _namedAnchors.end() || it->second.id < 0)
			Message::wstream(true) << "reference to undefined anchor '" << name << "'\n";
		else {
			int id = it->second.id;
			uri = "#loc"+XMLString(id);
			if (actions.getCurrentPageNumber() != it->second.pageno) {
				ostringstream oss;
				oss << actions.getSVGFilePath(it->second.pageno).relative() << uri;
				uri = oss.str();
			}
		}
	}
	if (!_base.empty() && uri.find("://") != string::npos) {
		if (*_base.rbegin() != '/' && uri[0] != '/' && uri[0] != '#')
			uri = "/" + uri;
		uri = _base + uri;
	}
	auto anchorNode = util::make_unique<XMLElement>("a");
	anchorNode->addAttribute("xlink:href", uri);
	anchorNode->addAttribute("xlink:title", XMLString(name.empty() ? uri : name, false));
	actions.svgTree().pushPageContext(std::move(anchorNode));
	actions.bbox("{anchor}", true);  // start computing the bounding box of the linked area
	_depthThreshold = actions.getDVIStackDepth();
	_anchorType = AnchorType::HREF;
}


void HyperlinkManager::closeAnchor (SpecialActions &actions) {
	if (_anchorType == AnchorType::HREF) {
		markLinkedBox(actions);
		actions.svgTree().popPageContext();
		_depthThreshold = 0;
	}
	_anchorType = AnchorType::NONE;
}


void HyperlinkManager::checkNewLine (SpecialActions &actions) {
	if (_anchorType == AnchorType::NONE)
		return;
	// Start a new box if the current depth of the DVI stack underruns
	// the initial threshold which indicates a line break.
	if (actions.getDVIStackDepth() < _depthThreshold) {
		markLinkedBox(actions);
		_depthThreshold = actions.getDVIStackDepth();
		actions.bbox("{anchor}", true);  // start a new box on the new line
	}
}


/** Marks a single rectangular area of the linked part of the document with a line or
 *  a box so that it's noticeable by the user. Additionally, an invisible rectangle is
 *  placed over this area in order to avoid flickering of the mouse cursor when moving
 *  it over the hyperlinked area. */
void HyperlinkManager::markLinkedBox (SpecialActions &actions) {
	const BoundingBox &bbox = actions.bbox("{anchor}");
	if (bbox.width() > 0 && bbox.height() > 0) {  // does the bounding box extend in both dimensions?
		if (MARKER_TYPE != MarkerType::NONE) {
			const double linewidth = _linewidth >= 0 ? _linewidth : min(0.5, bbox.height()/15);
			auto rect = util::make_unique<XMLElement>("rect");
			double x = bbox.minX();
			double y = bbox.maxY()+linewidth;
			double w = bbox.width();
			double h = linewidth;
			const Color linecolor = COLORSOURCE == ColorSource::DEFAULT ? actions.getColor() : LINK_LINECOLOR;
			if (MARKER_TYPE == MarkerType::LINE)
				rect->addAttribute("fill", linecolor.svgColorString());
			else {
				const double offset = _linewidth < 0 ? linewidth : 0 ;
				x -= offset;
				y = bbox.minY()-offset;
				w += 2*offset;
				h += bbox.height()+offset;
				if (MARKER_TYPE == MarkerType::BGCOLOR) {
					rect->addAttribute("fill", LINK_BGCOLOR.svgColorString());
					if (COLORSOURCE != ColorSource::DEFAULT) {
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
			actions.svgTree().prependToPage(std::move(rect));
			if (MARKER_TYPE == MarkerType::BOX || MARKER_TYPE == MarkerType::BGCOLOR) {
				// slightly enlarge the boxed area
				x -= linewidth/2;
				y -= linewidth/2;
				w += linewidth;
				h += linewidth;
			}
			actions.embed(BoundingBox(x, y, x+w, y+h));
		}
		// Create an invisible rectangle around the linked area so that it's easier to access.
		// This is only necessary when using paths rather than real text elements together with fonts.
		if (!SVGTree::USE_FONTS) {
			auto rect = util::make_unique<XMLElement>("rect");
			rect->addAttribute("x", bbox.minX());
			rect->addAttribute("y", bbox.minY());
			rect->addAttribute("width", bbox.width());
			rect->addAttribute("height", bbox.height());
			rect->addAttribute("fill", "white");
			rect->addAttribute("fill-opacity", 0);
			actions.svgTree().appendToPage(std::move(rect));
		}
	}
}


// Creates SVG views for all collected named anchors defined on a given page.
void HyperlinkManager::createViews (unsigned pageno, SpecialActions &actions) {
	const BoundingBox &pagebox = actions.bbox();
	for (auto &stranchorpair : _namedAnchors) {
		if (stranchorpair.second.pageno == pageno && stranchorpair.second.referenced) {  // current anchor referenced?
			ostringstream oss;
			oss << pagebox.minX() << ' ' << stranchorpair.second.pos << ' '
				 << pagebox.width() << ' ' << pagebox.height();
			auto view = util::make_unique<XMLElement>("view");
			view->addAttribute("id", "loc"+XMLString(stranchorpair.second.id));
			view->addAttribute("viewBox", oss.str());
			actions.svgTree().appendToDefs(std::move(view));
		}
	}
	closeAnchor(actions);
}


/** Sets the appearance of the link markers.
 *  @param[in] marker string specifying the marker (format: type[:linecolor])
 *  @return true on success */
bool HyperlinkManager::setLinkMarker (const string &marker) {
	string type;  // "none", "box", "line", or a background color specifier
	string color; // optional line color specifier
	size_t seppos = marker.find(':');
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
	COLORSOURCE = ColorSource::DEFAULT;
	if (MARKER_TYPE != MarkerType::NONE && !color.empty()) {
		if (!LINK_LINECOLOR.setPSName(color, false))
			return false;
		COLORSOURCE = ColorSource::LINKMARKER;
	}
	return true;
}


void HyperlinkManager::setDefaultLinkColor (Color color) {
	if (COLORSOURCE != ColorSource::LINKMARKER) {
		COLORSOURCE = ColorSource::STATIC;
		LINK_LINECOLOR = color;
	}
}