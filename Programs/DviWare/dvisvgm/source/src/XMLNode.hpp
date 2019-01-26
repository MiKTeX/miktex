/*************************************************************************
** XMLNode.hpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2019 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef XMLNODE_HPP
#define XMLNODE_HPP

#include <deque>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "utility.hpp"


class XMLNode {
	public:
		virtual ~XMLNode () =default;
		virtual std::unique_ptr<XMLNode> clone () const =0;
		virtual void clear () =0;
		virtual std::ostream& write (std::ostream &os) const =0;
};


class XMLElementNode : public XMLNode {
	public:
		using AttribMap = std::map<std::string,std::string>;
		using ChildList = std::deque<std::unique_ptr<XMLNode>>;

	public:
		XMLElementNode (const std::string &name);
		XMLElementNode (const XMLElementNode &node);
		XMLElementNode (XMLElementNode &&node);
		std::unique_ptr<XMLNode> clone () const override {return util::make_unique<XMLElementNode>(*this);}
		void clear () override;
		void addAttribute (const std::string &name, const std::string &value);
		void addAttribute (const std::string &name, double value);
		XMLNode* append (std::unique_ptr<XMLNode> &&child);
		XMLNode* append (const std::string &str);
		XMLNode* prepend (std::unique_ptr<XMLNode> &&child);
		void remove (const XMLNode *child);
		bool insertAfter (std::unique_ptr<XMLNode> &&child, XMLNode *sibling);
		bool insertBefore (std::unique_ptr<XMLNode> &&child, XMLNode *sibling);
		bool hasAttribute (const std::string &name) const;
		const char* getAttributeValue (const std::string &name) const;
		bool getDescendants (const char *name, const char *attrName, std::vector<XMLElementNode*> &descendants) const;
		XMLElementNode* getFirstDescendant (const char *name, const char *attrName, const char *attrValue) const;
		std::ostream& write (std::ostream &os) const override;
		bool empty () const                  {return _children.empty();}
		const ChildList& children () const   {return _children;}
		const std::string& getName () const  {return _name;}

	private:
		std::string _name;     // element name (<name a1="v1" .. an="vn">...</name>)
		AttribMap _attributes;
		ChildList _children;   // child nodes
};


class XMLTextNode : public XMLNode {
	public:
		XMLTextNode (const std::string &str) : _text(str) {}
		XMLTextNode (std::string &&str) : _text(std::move(str)) {}
		std::unique_ptr<XMLNode> clone () const override {return util::make_unique<XMLTextNode>(*this);}
		void clear () override {_text.clear();}
		void append (std::unique_ptr<XMLNode> &&node);
		void append (std::unique_ptr<XMLTextNode> &&node);
		void append (const std::string &str);
		void prepend (std::unique_ptr<XMLNode> &&node);
		std::ostream& write (std::ostream &os) const override {return os << _text;}
		const std::string& getText () const {return _text;}

	private:
		std::string _text;
};


class XMLCommentNode : public XMLNode {
	public:
		XMLCommentNode (const std::string &str) : _text(str) {}
		XMLCommentNode (std::string &&str) : _text(std::move(str)) {}
		std::unique_ptr<XMLNode> clone () const override {return util::make_unique<XMLCommentNode>(*this);}
		void clear () override {_text.clear();}
		std::ostream& write (std::ostream &os) const override {return os << "<!--" << _text << "-->";}

	private:
		std::string _text;
};


class XMLCDataNode : public XMLNode {
	public:
		XMLCDataNode () =default;
		XMLCDataNode (const std::string &d) : _data(d) {}
		XMLCDataNode (std::string &&d) : _data(std::move(d)) {}
		std::unique_ptr<XMLNode> clone () const override {return util::make_unique<XMLCDataNode>(*this);}
		void clear () override                {_data.clear();}
		void append (std::string &&str);
		std::ostream& write (std::ostream &os) const override;

	private:
		std::string _data;
};


inline std::ostream& operator << (std::ostream &os, const XMLElementNode &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLTextNode &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLCommentNode &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLCDataNode &node) {return node.write(os);}

#endif
