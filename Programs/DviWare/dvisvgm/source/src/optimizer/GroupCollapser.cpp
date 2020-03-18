/*************************************************************************
** GroupCollapser.cpp                                                   **
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

#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include "AttributeExtractor.hpp"
#include "GroupCollapser.hpp"
#include "../XMLNode.hpp"

using namespace std;


const char* GroupCollapser::info () const {
	return "join nested group elements";
}


/** Checks if there's only a single child element and optional whitespace
 *  siblings in a given element.
 *  @param[in] elem element to check
 *  @return pointer to the only child element or nullptr */
static XMLElement* only_child_element (XMLElement *elem) {
	XMLElement *firstChildElement=nullptr;
	for (XMLNode *child : *elem) {
		if (XMLElement *childElement = child->toElement()) {
			if (firstChildElement)
				return nullptr;
			firstChildElement = childElement;
		}
		else if (!child->toWSNode())
			return nullptr;
	}
	return firstChildElement;
}


/** Removes all whitespace child nodes from a given element. */
static void remove_ws_nodes (XMLElement *elem) {
	XMLNode *node = elem->firstChild();
	while (node) {
		if (!node->toWSNode())
			node = node->next();
		else {
			XMLNode *next = node->next();
			XMLElement::remove(node);
			node = next;
		}
	}
}


/** Recursively removes all redundant group elements from the given context element
 *  and moves their attributes to the corresponding parent element.
 *  @param[in] context root of the subtree to process */
void GroupCollapser::execute (XMLElement *context) {
	if (!context)
		return;
	XMLNode *node=context->firstChild();
	while (node) {
		XMLNode *next = node->next();  // keep safe pointer to next node
		if (XMLElement *elem = node->toElement())
			execute(elem);
		node = next;
	}
	if (context->name() == "g" && context->attributes().empty()) {
		// unwrap group without attributes
		remove_ws_nodes(context);
		XMLElement::unwrap(context);
	}
	else {
		XMLElement *child = only_child_element(context);
		if (child && collapsible(*context)) {
			if (child->name() == "g" && unwrappable(*child, *context) && moveAttributes(*child, *context)) {
				remove_ws_nodes(context);
				XMLElement::unwrap(child);
			}
		}
	}
}


/** Moves all attributes from an element to another one. Attributes already
 *  present in the destination element are overwritten or combined.
 *  @param[in] source element the attributes are taken from
 *  @param[in] dest element that receives the attributes
 *  @return true if all attributes have been moved */
bool GroupCollapser::moveAttributes (XMLElement &source, XMLElement &dest) {
	vector<string> movedAttributes;
	for (const XMLElement::Attribute &attr : source.attributes()) {
		if (attr.name == "transform") {
			string transform;
			if (const char *destvalue = dest.getAttributeValue("transform"))
				transform = destvalue+attr.value;
			else
				transform = attr.value;
			dest.addAttribute("transform", transform);
			movedAttributes.emplace_back("transform");
		}
		else if (AttributeExtractor::inheritable(attr)) {
			dest.addAttribute(attr.name, attr.value);
			movedAttributes.emplace_back(attr.name);
		}
	}
	for (const string &attrname : movedAttributes)
		source.removeAttribute(attrname);
	return source.attributes().empty();
}


/** Returns true if a given element is allowed to take the inheritable attributes
 *  and children of a child group without changing the semantics.
 *  @param[in] element group element to check */
bool GroupCollapser::collapsible (const XMLElement &element) {
	// the 'fill' attribute of animation elements has different semantics than
	// that of graphics elements => don't collapse them
	static const char *names[] = {
		"animate", "animateColor", "animateMotion", "animateTransform", "set"
	};
	auto it = find_if(begin(names), end(names), [&](const string &name) {
		return element.name() == name;
	});
	return it == end(names);
}


/** Returns true if a given group element is allowed to be unwrapped, i.e. its
 *  attributes and children can be moved to the parent without changing the semantics.
 *  @param[in] source element whose children and attributes should be moved
 *  @param[in] dest element that should receive the children and attributes */
bool GroupCollapser::unwrappable (const XMLElement &source, const XMLElement &dest) {
	// check for colliding clip-path attributes
	if (const char *cp1 = source.getAttributeValue("clip-path")) {
		if (const char *cp2 = dest.getAttributeValue("clip-path")) {
			if (string(cp1) != cp2)
				return false;
		}
	}
	// these attributes prevent a group from being unwrapped
	static const char *attribs[] = {
		"class", "id", "filter", "mask", "style"
	};
	auto it = find_if(begin(attribs), end(attribs), [&](const string &name) {
		return source.hasAttribute(name) || dest.hasAttribute(name);
	});
	return it == end(attribs);
}
