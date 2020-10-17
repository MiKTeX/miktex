/*************************************************************************
** XMLNode.cpp                                                          **
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
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include "FileSystem.hpp"
#include "utility.hpp"
#include "XMLNode.hpp"
#include "XMLString.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;

bool XMLNode::KEEP_ENCODED_FILES=false;
bool XMLElement::WRITE_NEWLINES=true;


/** Inserts a sibling node after this one.
 *  @param[in] node node to insert
 *  @return raw pointer to inserted node */
XMLNode* XMLNode::insertNext (unique_ptr<XMLNode> node) {
	if (_next) {
		_next->_prev = node.get();
		node->_next = std::move(_next);
	}
	node->_prev = this;
	node->_parent = _parent;
	_next = std::move(node);
	return _next.get();
}


/** Removes the following sibling node of this one.
 *  @return pointer to the removed node */
unique_ptr<XMLNode> XMLNode::removeNext () {
	unique_ptr<XMLNode> oldnext = std::move(_next);
	if (oldnext) {
		oldnext->_parent = oldnext->_prev = nullptr;
		if ((_next = std::move(oldnext->_next))) {
			_next->_prev = this;
			oldnext->_next.reset();
		}
	}
	return oldnext;
}

/////////////////////////////////////////////////////////////////////

XMLElement::XMLElement (string name) : _name(std::move(name)) {
}


XMLElement::XMLElement (const XMLElement &node)
	: XMLNode(node), _name(node._name), _attributes(node._attributes)
{
	for (XMLNode *child=node._firstChild.get(); child; child = child->next())
		insertLast(child->clone());
}


XMLElement::XMLElement (XMLElement &&node) noexcept
	: XMLNode(std::move(node)),
	_name(std::move(node._name)),
	_attributes(std::move(node._attributes)),
	_firstChild(std::move(node._firstChild)),
	_lastChild(node._lastChild)
{
}


XMLElement::~XMLElement () {
	// explicitly remove child nodes by iteration to prevent deep recursion
	unique_ptr<XMLNode> child = std::move(_firstChild);
	while (child && child->next())
		child->removeNext();
}


/** Removes all attributes and children. */
void XMLElement::clear () {
	_attributes.clear();
	_firstChild.reset();
	_lastChild = nullptr;
}


void XMLElement::addAttribute (const string &name, const string &value) {
	if (Attribute *attr = getAttribute(name))
		attr->value = value;
	else
		_attributes.emplace_back(Attribute(name, value));
}


void XMLElement::addAttribute (const string &name, double value) {
	addAttribute(name, XMLString(value));
}


void XMLElement::removeAttribute (const std::string &name) {
	_attributes.erase(find_if(_attributes.begin(), _attributes.end(), [&](const Attribute &attr) {
		return attr.name == name;
	}));
}


/** Inserts a new last child node and returns a raw pointer to it. */
XMLNode* XMLElement::insertLast (unique_ptr<XMLNode> child) {
	if (!child)
		return nullptr;
	child->parent(this);
	if (!empty())
		_lastChild = _lastChild->insertNext(std::move(child));
	else {
		_firstChild = std::move(child);
		_lastChild = _firstChild.get();
	}
	return _lastChild;
}


/** Inserts a new first child node and returns a raw pointer to it. */
XMLNode* XMLElement::insertFirst (unique_ptr<XMLNode> child) {
	if (!child)
		return nullptr;
	child->parent(this);
	if (empty()) {
		_firstChild = std::move(child);
		_lastChild = _firstChild.get();
	}
	else {
		child->insertNext(std::move(_firstChild));
		_firstChild = std::move(child);
	}
	return _firstChild.get();
}


/** Appends a child node to this element. The element also takes the ownership of the child.
 *  @param[in] child node to be appended
 *  @return raw pointer to the appended child node */
XMLNode* XMLElement::append (unique_ptr<XMLNode> child) {
	if (!child)
		return nullptr;
	XMLText *textNode1 = child->toText();
	if (!textNode1 || empty())
		insertLast(std::move(child));
	else {
		if (XMLText *textNode2 = _lastChild->toText())
			textNode2->append(util::static_unique_ptr_cast<XMLText>(std::move(child)));  // merge two consecutive text nodes
		else
			insertLast(std::move(child));
	}
	return _lastChild;
}


/** Appends a string to this element. If the last child is a text node, the string is
 *  appended there, otherwise a new text node is created.
 *  @param[in] str string to be appended
 *  @return raw pointer to the text node the string was appended to */
XMLNode* XMLElement::append (const string &str) {
	XMLText *last;
	if (!empty() && (last = _lastChild->toText()))
		last->append(str);
	else
		insertLast(util::make_unique<XMLText>(str));
	return _lastChild;
}


/** Prepends a child node to this element. The element also takes the ownership of the child.
 *  @param[in] child node to be prepended
 *  @return raw pointer to the prepended child node */
XMLNode* XMLElement::prepend (unique_ptr<XMLNode> child) {
	if (!child)
		return nullptr;
	XMLText *textNode1 = child->toText();
	if (textNode1 && !empty()) {
		if (XMLText *textNode2 = _firstChild->toText()) {
			textNode2->prepend(util::static_unique_ptr_cast<XMLText>(std::move(child)));  // merge two consecutive text nodes
			return textNode2;
		}
	}
	insertFirst(std::move(child));
	return _firstChild.get();
}


/** Inserts a new child node before a given child node already present. The latter
 *  will be the following sibling of the node to be inserted. If there's no such
 *  node present, nothing is inserted.
 *  @param[in] child node to be inserted
 *  @param[in] sibling following sibling of 'child'
 *  @return raw pointer to inserted node */
XMLNode* XMLElement::insertBefore (unique_ptr<XMLNode> child, XMLNode *sibling) {
	if (!child || (sibling && sibling->parent() != this))
		return nullptr;
	if (!sibling)
		return insertLast(std::move(child));
	if (sibling == _firstChild.get())
		return insertFirst(std::move(child));
	return sibling->prev()->insertNext(std::move(child));
}


/** Inserts a new child node after a given child node already present. The latter
 *  will be the preceding sibling of the node to be inserted. If there's no such
 *  node present, nothing is inserted.
 *  @param[in] child node to be inserted
 *  @param[in] sibling preceding sibling of 'child'
 *  @return raw pointer to inserted node */
XMLNode* XMLElement::insertAfter (unique_ptr<XMLNode> child, XMLNode *sibling) {
	if (!child || (sibling && sibling->parent() != this))
		return nullptr;
	if (!sibling)
		return insertFirst(std::move(child));
	if (sibling == _lastChild)
		return insertLast(std::move(child));
	return sibling->insertNext(std::move(child));
}


/** Moves a sequence of child nodes to a new element of a given name and inserts
 *  this (wrapper) element at the former position of the first node of the sequence.
 *  Example: wrap 3 child nodes of element a with b:
 *  <a>text1<c/>text2<d/></a> => <a>text1<b><c/>text2<d/></b></a>
 *  @param[in] first first node to wrap
 *  @param[in] last last node to wrap (or nullptr if all following siblings of 'first' are to be wrapped)
 *  @param[in] name name of the wrapper element to be created
 *  @return raw pointer to the new wrapper element */
XMLElement* XMLElement::wrap (XMLNode *first, XMLNode *last, const string &name) {
	if (!first || !first->parent() || (last && first->parent() != last->parent()))
		return nullptr;
	XMLElement *parent = first->parent()->toElement();
	XMLNode *prev = first->prev();
	auto wrapper = util::make_unique<XMLElement>(name);
	if (last)
		last = last->next();
	XMLNode *child = first;
	while (child && child != last) {
		XMLNode *next = child->next();
		wrapper->insertLast(detach(child));
		child = next;
	}
	XMLElement *ret = wrapper.get();
	if (prev)
		parent->insertAfter(std::move(wrapper), prev);
	else
		parent->insertFirst(std::move(wrapper));
	return ret;
}


/** Moves all child nodes C1,...,Cn of a given element E to its parent and
 *  removes E afterwards, so that C1 is located at the former position of E
 *  followed by C2,...,Cn.
 *  Example: unwrap a child element b of a:
 *  <a>text1<b><c/>text2<d/></b></a> => <a>text1<c/>text2<d/></a>
 *  @param[in] child child element to unwrap
 *  @return raw pointer to the first node C1 of the unwrapped sequence or nullptr if element was empty */
XMLNode* XMLElement::unwrap (XMLElement *element) {
	if (!element || !element->parent())
		return nullptr;
	XMLElement *parent = element->parent()->toElement();
	XMLNode *prev = element->prev();
	auto detachedElement = util::static_unique_ptr_cast<XMLElement>(detach(element));
	if (detachedElement->empty())
		return nullptr;
	XMLNode *firstChild = detachedElement->firstChild();
	while (auto child = detach(detachedElement->firstChild()))
		prev = parent->insertAfter(std::move(child), prev);
	return firstChild;
}


/** Isolates a node and its descendants from a subtree.
 *  @param[in] node raw pointer to node to be detached
 *  @return unique pointer to the detached node. */
unique_ptr<XMLNode> XMLElement::detach (XMLNode *node) {
	unique_ptr<XMLNode> uniqueNode;
	if (node && node->parent()) {
		XMLElement *parent = node->parent()->toElement();
		if (node == parent->_lastChild)
			parent->_lastChild = node->prev();
		if (node != parent->firstChild())
			uniqueNode = node->prev()->removeNext();
		else {
			uniqueNode = std::move(parent->_firstChild);
			if ((parent->_firstChild = std::move(uniqueNode->_next)))
				parent->_firstChild->prev(nullptr);
		}
		node->parent(nullptr);
	}
	return uniqueNode;
}


/** Gets all descendant elements with a given name and given attribute.
 *  @param[in] name name of elements to find
 *  @param[in] attrName name of attribute to find
 *  @param[out] descendants all elements found
 *  @return true if at least one element was found  */
bool XMLElement::getDescendants (const char *name, const char *attrName, vector<XMLElement*> &descendants) const {
	for (XMLNode *child = _firstChild.get(); child; child = child->next()) {
		if (XMLElement *elem = child->toElement()) {
			if (!name || !name[0] || (name[0] != '!' && elem->name() == name) || (name[0] == '!' && elem->name() != name+1)) {
				if (!attrName || elem->hasAttribute(attrName))
					descendants.push_back(elem);
			}
			elem->getDescendants(name, attrName, descendants);
		}
	}
	return !descendants.empty();
}


/** Returns the first descendant element that matches the given properties in depth first order.
 *  @param[in] name element name; if 0, all elements are taken into account
 *  @param[in] attrName if not 0, only elements with an attribute of this name are considered
 *  @param[in] attrValue if not 0, only elements with attribute attrName="attrValue" are considered
 *  @return pointer to the found element or 0 */
XMLElement* XMLElement::getFirstDescendant (const char *name, const char *attrName, const char *attrValue) const {
	for (XMLNode *child = _firstChild.get(); child; child = child->next()) {
		if (XMLElement *elem = child->toElement()) {
			if (!name || elem->name() == name) {
				const char *value;
				if (!attrName || (((value = elem->getAttributeValue(attrName)) != nullptr) && (!attrValue || string(value) == attrValue)))
					return elem;
			}
			if (XMLElement *descendant = elem->getFirstDescendant(name, attrName, attrValue))
				return descendant;
		}
	}
	return nullptr;
}


ostream& XMLElement::write (ostream &os) const {
	os << '<' << _name;
	for (const auto &attrib : _attributes) {
		os << ' ';
		if (attrib.name.front() != '@')
			os << attrib.name << "='" << attrib.value << '\'';
		else {
			os << attrib.name.substr(1) << "='";
			size_t pos = attrib.value.find("base64,");
			if (pos == string::npos)
				os << attrib.value;
			else {
				os << attrib.value.substr(0, pos+7);
				string fname = attrib.value.substr(pos+7);
#if defined(MIKTEX_WINDOWS)
				ifstream ifs(EXPATH_(fname), ios::binary);
#else
				ifstream ifs(fname, ios::binary);
#endif
				if (ifs) {
					os << '\n';
					util::base64_copy(ifs, os, 200);
					ifs.close();
					if (!KEEP_ENCODED_FILES)
						FileSystem::remove(fname);
				}
			}
			os << "'";
		}
	}
	if (empty())
		os << "/>";
	else {
		os << '>';
		// Insert newlines around children except text nodes. According to the
		// SVG specification, pure whitespace nodes are ignored by the SVG renderer.
		if (WRITE_NEWLINES && !_firstChild->toText())
			os << '\n';
		for (XMLNode *child = _firstChild.get(); child; child = child->next()) {
			child->write(os);
			if (!child->toText()) {
				if (WRITE_NEWLINES && (!child->next() || !child->next()->toText()))
					os << '\n';
			}
		}
		os << "</" << _name << '>';
	}
	return os;
}


/** Returns true if this element has an attribute of given name. */
bool XMLElement::hasAttribute (const string &name) const {
	return getAttribute(name) != nullptr;
}


/** Returns the value of an attribute.
 *  @param[in] name name of attribute
 *  @return attribute value or 0 if attribute doesn't exist */
const char* XMLElement::getAttributeValue (const std::string& name) const {
	if (const Attribute *attr = getAttribute(name))
		return attr->value.c_str();
	return nullptr;
}


XMLElement::Attribute* XMLElement::getAttribute (const string &name) {
	auto it = find_if(_attributes.begin(), _attributes.end(), [&](const Attribute &attr) {
		return attr.name == name;
	});
	return it != _attributes.end() ? &(*it) : nullptr;
}


const XMLElement::Attribute* XMLElement::getAttribute (const string &name) const {
	auto it = find_if(_attributes.begin(), _attributes.end(), [&](const Attribute &attr) {
		return attr.name == name;
	});
	return it != _attributes.end() ? &(*it) : nullptr;
}


//////////////////////

void XMLText::append (unique_ptr<XMLNode> node) {
	if (!node)
		return;
	if (node->toText())
		append(util::static_unique_ptr_cast<XMLText>(std::move(node)));
	else {
		// append text representation of the node
		ostringstream oss;
		node->write(oss);
		append(XMLString(oss.str()));
	}
}


void XMLText::append (unique_ptr<XMLText> node) {
	if (node)
		_text += node->_text;
}


void XMLText::append (const string &str) {
	_text += str;
}


void XMLText::prepend (unique_ptr<XMLNode> node) {
	if (XMLText *textNode = node->toText())
		_text = textNode->_text + _text;
}


const XMLText* XMLText::toWSNode () const {
	return _text.find_first_not_of(" \t\n\r") == string::npos ? this : nullptr;
}

/////////////////////////////////////////////////////////////////////

ostream& XMLCData::write (ostream &os) const {
	if (!_data.empty())
		os << "<![CDATA[" << _data << "]]>";
	return os;
}


void XMLCData::append (string &&str) {
	if (_data.empty())
		_data = move(str);
	else
		_data += str;
}
