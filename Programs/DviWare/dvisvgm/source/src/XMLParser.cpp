/*************************************************************************
** XMLParser.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include "InputReader.hpp"
#include "XMLParser.hpp"

using namespace std;

XMLElement* XMLParser::setRootElement (XMLElement *root) {
	_elementStack.clear();
	if (root)
		_elementStack.push_back(root);
	else {
		_root = util::make_unique<XMLElement>("root");
		_elementStack.push_back(_root.get());
	}
	return _elementStack.back();
}


/** Parses an XML fragment from an input stream. */
void XMLParser::parse (istream &is) {
	while (is) {
		string buf;
		buf.resize(1024);
		is.read(&buf[0], 1024);
		buf.resize(is.gcount());
		parse(std::move(buf));
	}
	finish();
}


static string::size_type find_end_of_tag (const string &str, string::size_type startpos) {
	char attrval_delim = 0;
	bool expect_attrval = false;
	for (auto i=startpos; i < str.length(); i++) {
		if (attrval_delim) {  // inside attrubute value?
			if (str[i] == attrval_delim)  // end of attribute value?
				attrval_delim = 0;
		}
		else if (str[i] == '>')
			return i;
		else if (str[i] == '"' || str[i] == '\'') { // start of attribute value?
			if (expect_attrval)
				attrval_delim = str[i];
			else {
				ostringstream oss;
				oss << "misplaced " << str[i] << " inside tag";
				throw XMLParserException(oss.str());
			}
		}
		else if (str[i] == '=') {
			expect_attrval = true;
			continue;
		}
		else if (str[i] == '<')
			throw XMLParserException("invalid '<' inside tag");
		expect_attrval = false;
	}
	return string::npos;
}


/** Parses a fragment of XML code, creates corresponding XML nodes and adds them
 *  to an SVG tree. The code may be split and processed by several calls of this
 *  function. Incomplete chunks that can't be processed yet are stored and picked
 *  up again together with the next incoming XML fragment. If a call of this function
 *  is supposed to finish the parsing of an XML subtree, parameter 'finish' must be set.
 *  @param[in] xml XML fragment to parse
 *  @param[in] finish if true, no more XML is expected and parsing is finished */
void XMLParser::parse (string xml, bool finish) {
	if (_elementStack.empty())  // no root element set?
		return;
	// collect/extract an XML fragment that only contains complete tags
	// incomplete tags are held back
	if (_xmlbuf.empty())
		_xmlbuf.assign(std::move(xml));
	else
		_xmlbuf.append(xml);
	string::size_type left=0;
	try {
		while (left != string::npos) {
			auto right = _xmlbuf.find('<', left);
			if (left < right && left < _xmlbuf.length())  {// plain text found?
				string text = (right == string::npos ? _xmlbuf.substr(left) : _xmlbuf.substr(left, right-left));
				appendNode(util::make_unique<XMLText>(std::move(text)));
			}
			if (right != string::npos) {
				left = right;
				if (_xmlbuf.compare(left, 9, "<![CDATA[") == 0) {
					right = _xmlbuf.find("]]>", left+9);
					if (right == string::npos) {
						if (finish) throw XMLParserException("expected ']]>' at end of CDATA block");
						break;
					}
					appendNode(util::make_unique<XMLCData>(_xmlbuf.substr(left+9, right-left-9)));
					right += 2;
				}
				else if (_xmlbuf.compare(left, 4, "<!--") == 0) {
					right = _xmlbuf.find("-->", left+4);
					if (right == string::npos) {
						if (finish) throw XMLParserException("expected '-->' at end of comment");
						break;
					}
					appendNode(util::make_unique<XMLComment>(_xmlbuf.substr(left+4, right-left-4)));
					right += 2;
				}
				else if (_xmlbuf.compare(left, 2, "<?") == 0) {
					right = _xmlbuf.find("?>", left+2);
					if (right == string::npos) {
						if (finish) throw XMLParserException("expected '?>' at end of processing instruction");
						break;
					}
					appendNode(util::make_unique<XMLText>(_xmlbuf.substr(left, right-left+2)));
					right++;
				}
				else if (_xmlbuf.compare(left, 2, "</") == 0) {
					right = _xmlbuf.find('>', left+2);
					if (right == string::npos) {
						if (finish) throw XMLParserException("missing '>' at end of closing XML tag");
						break;
					}
					closeElement(_xmlbuf.substr(left+2, right-left-2));
				}
				else {
					right = find_end_of_tag(_xmlbuf, left+1);
					if (right == string::npos) {
						if (finish)	throw XMLParserException("missing '>' or '/>' at end of opening XML tag");
						break;
					}
					openElement(_xmlbuf.substr(left+1, right-left-1));
				}
			}
			left = right;
			if (right != string::npos)
				left++;
		}
	}
	catch (const XMLParserException &e) {
		_error = true;
		throw;
	}
	if (left == string::npos)
		_xmlbuf.clear();
	else
		_xmlbuf.erase(0, left);
}


void XMLParser::appendNode (unique_ptr<XMLNode> node) {
	context()->append(std::move(node));
}


XMLElement* XMLParser::finishPushContext (unique_ptr<XMLElement> elem) {
	XMLElement *elemPtr = elem.get();
	context()->append(std::move(elem));
	return elemPtr;
}


/** Processes an opening element tag.
 *  @param[in] tag tag without leading and trailing angle brackets */
XMLElement* XMLParser::openElement (const string &tag) {
	StringInputBuffer ib(tag);
	BufferInputReader ir(ib);
	string name = ir.getString("/ \t\n\r");
	ir.skipSpace();
	unique_ptr<XMLElement> elemNode{createElementPtr(name)};
	map<string, string> attribs;
	if (ir.parseAttributes(attribs, true, "\"'")) {
		for (const auto &attrpair : attribs)
			elemNode->addAttribute(attrpair.first, attrpair.second);
	}
	ir.skipSpace();
	XMLElement *elemPtr = elemNode.get();
	if (ir.peek() == '/')       // end of empty element tag
		appendNode(std::move(elemNode));
	else if (ir.peek() < 0)   // end of opening tag
		_elementStack.push_back(finishPushContext(std::move(elemNode)));
	else
		throw XMLParserException("'>' or '/>' expected at end of opening tag <"+name);
	if (_notifyElementOpened)
		_notifyElementOpened(elemPtr);
	if (ir.peek() == '/' && _notifyElementClosed)
		_notifyElementClosed(elemPtr);
	return elemPtr;
}


/** Processes a closing element tag.
 *  @param[in] tag tag without leading and trailing angle brackets */
void XMLParser::closeElement (const string &tag) {
	StringInputBuffer ib(tag);
	BufferInputReader ir(ib);
	string name = ir.getString(" \t\n\r");
	ir.skipSpace();
	if (ir.peek() >= 0)
		throw XMLParserException("'>' expected at end of closing tag </"+name);
	if (_elementStack.size() <= 1)
		throw XMLParserException("spurious closing tag </" + name + ">");
	if (_elementStack.back()->name() != name)
		throw XMLParserException("expected </" + _elementStack.back()->name() + "> but found </" + name + ">");
	finishPopContext();
	if (_notifyElementClosed)
		_notifyElementClosed(_elementStack.back());
	_elementStack.pop_back();
}


/** Processes any remaining XML fragments, checks for missing closing tags,
 *  and resets the parser state. */
void XMLParser::finish () {
	if (!_xmlbuf.empty()) {
		if (!_error)
			parse("", true);
		_xmlbuf.clear();
	}
	string tags;
	while (_elementStack.size() > 1) {
		tags += "</" + _elementStack.back()->name() + ">, ";
		_elementStack.pop_back();
	}
	if (!tags.empty() && !_error) {
		tags.resize(tags.length()-2);  // drop trailing ", "
		throw XMLParserException("missing closing tag(s): " + tags);
	}
}


XMLElement* XMLParser::createElementPtr (std::string name) const {
	return new XMLElement(std::move(name));
}


void XMLParser::setNotifyFuncs (NotifyFunc notifyElementOpened, NotifyFunc notifyElementClosed) {
	_notifyElementOpened = std::move(notifyElementOpened);
	_notifyElementClosed = std::move(notifyElementClosed);
}
