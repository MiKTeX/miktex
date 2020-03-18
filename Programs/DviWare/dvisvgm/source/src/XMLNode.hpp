/*************************************************************************
** XMLNode.hpp                                                          **
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

#ifndef XMLNODE_HPP
#define XMLNODE_HPP

#include <deque>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "utility.hpp"

class XMLCData;
class XMLComment;
class XMLElement;
class XMLText;

class XMLNode {
	friend class XMLElement;

	template <typename T>
	T* cast (const T* (XMLNode::*func)() const) {
		return const_cast<T*>((const_cast<const XMLNode*>(this)->*func)());
	}

	public:
		XMLNode () =default;
		XMLNode (const XMLNode &node) : _next(nullptr) {}
		XMLNode (XMLNode &&node) noexcept : _parent(node._parent), _prev(node._prev), _next(std::move(node._next)) {}
		virtual ~XMLNode () =default;
		virtual std::unique_ptr<XMLNode> clone () const =0;
		virtual void clear () =0;
		virtual std::ostream& write (std::ostream &os) const =0;
		virtual const XMLElement* toElement () const {return nullptr;}
		virtual const XMLText* toText () const       {return nullptr;}
		virtual const XMLText* toWSNode () const     {return nullptr;}
		virtual const XMLComment* toComment () const {return nullptr;}
		virtual const XMLCData* toCData () const     {return nullptr;}
		XMLElement* toElement ()  {return cast<XMLElement>(&XMLNode::toElement);}
		XMLText* toText ()        {return cast<XMLText>(&XMLNode::toText);}
		XMLComment* toComment ()  {return cast<XMLComment>(&XMLNode::toComment);}
		XMLCData* toCData ()      {return cast<XMLCData>(&XMLNode::toCData);}
		XMLNode* parent () const  {return _parent;}
		XMLNode* prev () const    {return _prev;}
		XMLNode* next () const    {return _next.get();}

		static bool KEEP_ENCODED_FILES;

	protected:
		XMLNode* insertNext (std::unique_ptr<XMLNode> node);
		std::unique_ptr<XMLNode> removeNext ();
		void parent (XMLNode *p) {_parent = p;}
		void prev (XMLNode *p)   {_prev = p;}

	private:
		XMLNode *_parent=nullptr;  ///< pointer to parent node
		XMLNode *_prev=nullptr;    ///< pointer to preceding sibling
		std::unique_ptr<XMLNode> _next;  ///< pointer to next sibling (incl. ownership)
};


class XMLNodeIterator {
	public:
		XMLNodeIterator () =default;
		explicit XMLNodeIterator (XMLNode *curr) : _curr(curr) {}
		XMLNodeIterator& operator ++ ()   {_curr = _curr->next(); return *this;}
		XMLNodeIterator& operator -- ()   {_curr = _curr->prev(); return *this;}
		XMLNodeIterator operator ++ (int) {auto p=_curr; _curr = _curr->next(); return XMLNodeIterator(p);}
		XMLNodeIterator operator -- (int) {auto p=_curr; _curr = _curr->prev(); return XMLNodeIterator(p);}
		XMLNode* operator * ()     {return _curr;}
		XMLNode& operator -> ()    {return *_curr;}
		bool operator == (const XMLNodeIterator &it) const {return _curr == it._curr;}
		bool operator != (const XMLNodeIterator &it) const {return _curr != it._curr;}

	private:
		XMLNode *_curr=nullptr;
};


class ConstXMLNodeIterator {
	public:
		ConstXMLNodeIterator () =default;
		explicit ConstXMLNodeIterator (const XMLNode *curr) : _curr(curr) {}
		ConstXMLNodeIterator& operator ++ ()   {_curr = _curr->next(); return *this;}
		ConstXMLNodeIterator& operator -- ()   {_curr = _curr->prev(); return *this;}
		ConstXMLNodeIterator operator ++ (int) {auto p=_curr; _curr = _curr->next(); return ConstXMLNodeIterator(p);}
		ConstXMLNodeIterator operator -- (int) {auto p=_curr; _curr = _curr->prev(); return ConstXMLNodeIterator(p);}
		const XMLNode* operator * ()     {return _curr;}
		const XMLNode& operator -> ()    {return *_curr;}
		bool operator == (const ConstXMLNodeIterator &it) const {return _curr == it._curr;}
		bool operator != (const ConstXMLNodeIterator &it) const {return _curr != it._curr;}

	private:
		const XMLNode *_curr=nullptr;
};


class XMLElement : public XMLNode {
	public:
		struct Attribute {
			Attribute (std::string nam, std::string val) : name(std::move(nam)), value(std::move(val)) {}
			std::string name;
			std::string value;
		};
		using Attributes = std::vector<Attribute>;
		static bool WRITE_NEWLINES;  ///< insert line breaks after element tags?

	public:
		explicit XMLElement (std::string name);
		XMLElement (const XMLElement &node);
		XMLElement (XMLElement &&node) noexcept;
		~XMLElement ();
		std::unique_ptr<XMLNode> clone () const override {return util::make_unique<XMLElement>(*this);}
		void clear () override;
		void addAttribute (const std::string &name, const std::string &value);
		void addAttribute (const std::string &name, double value);
		void removeAttribute (const std::string &name);
		XMLNode* append (std::unique_ptr<XMLNode> child);
		XMLNode* append (const std::string &str);
		XMLNode* prepend (std::unique_ptr<XMLNode> child);
		XMLNode* insertAfter (std::unique_ptr<XMLNode> child, XMLNode *sibling);
		XMLNode* insertBefore (std::unique_ptr<XMLNode> child, XMLNode *sibling);
		bool hasAttribute (const std::string &name) const;
		const char* getAttributeValue (const std::string &name) const;
		bool getDescendants (const char *name, const char *attrName, std::vector<XMLElement*> &descendants) const;
		XMLElement* getFirstDescendant (const char *name, const char *attrName, const char *attrValue) const;
		XMLNode* firstChild () const {return _firstChild.get();}
		XMLNode* lastChild () const {return _lastChild;}
		std::ostream& write (std::ostream &os) const override;
		bool empty () const {return !_firstChild;}
		Attributes& attributes () {return _attributes;}
		const Attributes& attributes () const {return _attributes;}
		XMLNodeIterator begin () {return XMLNodeIterator(_firstChild.get());}
		XMLNodeIterator end () {return XMLNodeIterator(nullptr);}
		ConstXMLNodeIterator begin () const {return ConstXMLNodeIterator(_firstChild.get());}
		ConstXMLNodeIterator end () const {return ConstXMLNodeIterator(nullptr);}
		const std::string& name () const {return _name;}
		const XMLElement* toElement () const override {return this;}
		const Attribute* getAttribute (const std::string &name) const;

		static std::unique_ptr<XMLNode> remove (XMLNode *child);
		static XMLElement* wrap (XMLNode *first, XMLNode *last, const std::string &name);
		static XMLNode* unwrap (XMLElement *child);

	protected:
		Attribute* getAttribute (const std::string &name);
		XMLNode* insertFirst (std::unique_ptr<XMLNode> child);
		XMLNode* insertLast (std::unique_ptr<XMLNode> child);

	private:
		std::string _name;     // element name (<name a1="v1" .. an="vn">...</name>)
		std::vector<Attribute> _attributes;
		std::unique_ptr<XMLNode> _firstChild;  ///< pointer to first child node (incl. ownership)
		XMLNode *_lastChild=nullptr;  ///< pointer to last child node
};


class XMLText : public XMLNode {
	public:
		explicit XMLText (std::string str) : _text(std::move(str)) {}
		std::unique_ptr<XMLNode> clone () const override {return util::make_unique<XMLText>(*this);}
		void clear () override {_text.clear();}
		void append (std::unique_ptr<XMLNode> node);
		void append (std::unique_ptr<XMLText> node);
		void append (const std::string &str);
		void prepend (std::unique_ptr<XMLNode> node);
		std::ostream& write (std::ostream &os) const override {return os << _text;}
		const std::string& getText () const {return _text;}
		const XMLText* toText () const override {return this;}
		const XMLText* toWSNode () const override;

	private:
		std::string _text;
};


class XMLComment : public XMLNode {
	public:
		explicit XMLComment (std::string str) : _text(std::move(str)) {}
		std::unique_ptr<XMLNode> clone () const override {return util::make_unique<XMLComment>(*this);}
		void clear () override {_text.clear();}
		std::ostream& write (std::ostream &os) const override {return os << "<!--" << _text << "-->";}
		const XMLComment* toComment () const override {return this;}

	private:
		std::string _text;
};


class XMLCData : public XMLNode {
	public:
		XMLCData () =default;
		explicit XMLCData (std::string data) : _data(std::move(data)) {}
		std::unique_ptr<XMLNode> clone () const override {return util::make_unique<XMLCData>(*this);}
		void clear () override                {_data.clear();}
		void append (std::string &&str);
		std::ostream& write (std::ostream &os) const override;
		const XMLCData* toCData () const override {return this;}

	private:
		std::string _data;
};


inline std::ostream& operator << (std::ostream &os, const XMLElement &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLText &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLComment &node) {return node.write(os);}
inline std::ostream& operator << (std::ostream &os, const XMLCData &node) {return node.write(os);}

#endif
