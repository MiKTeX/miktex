/*************************************************************************
** TextSimplifier.cpp                                                   **
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

#include <vector>
#include "AttributeExtractor.hpp"
#include "TextSimplifier.hpp"
#include "../XMLNode.hpp"

using namespace std;


const char* TextSimplifier::info () const {
	return "merge data of tspans into enclosing text elements";
}


/** Returns all common inheritable attributes of multiple elements. */
static XMLElement::Attributes common_inheritable_attributes (const vector<XMLElement*> &elements) {
	bool intersected=false;
	XMLElement::Attributes commonAttribs;
	for (const XMLElement *elem : elements) {
		if (commonAttribs.empty()) {
			if (intersected)
				break;
			for (const auto attrib : elem->attributes()) {
				if (AttributeExtractor::inheritable(attrib))
					commonAttribs.push_back(attrib);
			}
		}
		else {
			for (auto it = commonAttribs.begin(); it != commonAttribs.end();) {
				auto *attrib = elem->getAttribute(it->name);
				if (!attrib || attrib->value != it->value)
					it = commonAttribs.erase(it);
				else
					++it;
			}
		}
		intersected = true;
	}
	return commonAttribs;
}


/** Returns all tspan elements of a text element if the latter doesn't contain
 *  any non-whitespace text nodes. Otherwise, the returned vector is empty.
 *  @param[in] textElement text element to check
 *  @return the tspan children of the text element */
static vector<XMLElement*> get_tspans (XMLElement *textElement) {
	vector<XMLElement*> tspans;
	bool failed=false;
	for (XMLNode *child : *textElement) {
		if (child->toWSNode() || child->toComment())
			continue;
		if (child->toText())
			failed = true;
		else if (XMLElement *childElement = child->toElement()) {
			if (childElement->name() != "tspan")
				failed = true;
			else
				tspans.push_back(childElement);
		}
		if (failed) {
			tspans.clear();
			break;
		}
	}
	return tspans;
}


void TextSimplifier::execute (XMLElement *context) {
	if (!context)
		return;
	if (context->name() == "text") {
		vector<XMLElement*> tspans = get_tspans(context);
		vector<XMLElement::Attribute> attribs = common_inheritable_attributes(tspans);
		if (!tspans.empty() && !attribs.empty()) {
			// move all common tspan attributes to the parent text element
			for (const XMLElement::Attribute &attr : attribs)
				context->addAttribute(attr.name, attr.value);
			// remove all common attributes from the tspan elements
			for (XMLElement *tspan : tspans) {
				for (const XMLElement::Attribute &attr : attribs)
					tspan->removeAttribute(attr.name);
				// unwrap the tspan if there are no remaining attributes
				if (tspan->attributes().empty())
					XMLElement::unwrap(tspan);
			}
		}
	}
	else {
		XMLNode *node = context->firstChild();
		while (node) {
			XMLNode *next = node->next();  // keep safe pointer to next node
			if (XMLElement *elem = node->toElement())
				execute(elem);
			node = next;
		}
	}
}
