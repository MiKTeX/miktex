/*************************************************************************
** XMLNode.h                                                            **
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

#ifndef DVISVGM_XMLNODE_H
#define DVISVGM_XMLNODE_H

#include <list>
#include <map>
#include <ostream>
#include <string>
#include <vector>


class XMLNode
{
	public:
		virtual ~XMLNode () {}
		virtual XMLNode* clone () const =0;
		virtual void clear () =0;
		virtual std::ostream& write (std::ostream &os) const =0;
};


class XMLElementNode : public XMLNode
{
	typedef std::map<std::string,std::string> AttribMap;
	typedef std::list<XMLNode*> ChildList;
	public:
		XMLElementNode (const std::string &name);
		XMLElementNode (const XMLElementNode &node);
		~XMLElementNode ();
		XMLElementNode* clone () const {return new XMLElementNode(*this);}
		void clear ();
		void addAttribute (const std::string &name, const std::string &value);
		void addAttribute (const std::string &name, double value);
		void append (XMLNode *child);
		void append (const std::string &str);
		void prepend (XMLNode *child);
		void remove (XMLNode *child)                 {_children.remove(child);}
		bool insertAfter (XMLNode *child, XMLNode *sibling);
		bool insertBefore (XMLNode *child, XMLNode *sibling);
		bool hasAttribute (const std::string &name) const;
		const char* getAttributeValue (const std::string &name) const;
		bool getDescendants (const char *name, const char *attrName, std::vector<XMLElementNode*> &descendants) const;
		XMLElementNode* getFirstDescendant (const char *name, const char *attrName, const char *attrValue) const;
		std::ostream& write (std::ostream &os) const;
		bool empty () const                          {return _children.empty();}
		const std::list<XMLNode*>& children () const {return _children;}
		const std::string& getName () const          {return _name;}

	private:
		std::string _name;     // element name (<name a1="v1" .. an="vn">...</name>)
		AttribMap _attributes;
		ChildList _children;   // child nodes
};


class XMLTextNode : public XMLNode
{
	public:
		XMLTextNode (const std::string &str) : _text(str) {}
		XMLTextNode* clone () const {return new XMLTextNode(*this);}
		void clear ()           {_text.clear();}
		void append (XMLNode *node);
		void append (XMLTextNode *node);
		void append (const std::string &str);
		void prepend (XMLNode *child);
		std::ostream& write (std::ostream &os) const {return os << _text;}
		const std::string& getText () const {return _text;}

	private:
		std::string _text;
};


class XMLCommentNode : public XMLNode
{
	public:
		XMLCommentNode (const std::string &str) : _text(str) {}
		XMLCommentNode* clone () const {return new XMLCommentNode(*this);}
		void clear () {_text.clear();}
		std::ostream& write (std::ostream &os) const {return os << "<!--" << _text << "-->";}

	private:
		std::string _text;
};


class XMLCDataNode : public XMLNode
{
	public:
		XMLCDataNode (const std::string &d) : _data(d) {}
		XMLCDataNode* clone () const {return new XMLCDataNode(*this);}
		void clear () {_data.clear();}
		std::ostream& write (std::ostream &os) const;

	private:
		std::string _data;
};


inline std::ostream& operator << (std::ostream &os, const XMLElementNode &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLTextNode &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLCommentNode &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLCDataNode &node) {return node.write(os);}

#endif
