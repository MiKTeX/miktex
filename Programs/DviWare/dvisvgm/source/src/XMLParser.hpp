/*************************************************************************
** XMLParser.hpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef XMLPARSER_HPP
#define XMLPARSER_HPP

#include <functional>
#include <string>
#include "MessageException.hpp"
#include "XMLNode.hpp"

struct XMLParserException : MessageException {
	explicit XMLParserException (const std::string &msg) : MessageException(msg) {}
};

class XMLParser {
	using ElementStack = std::vector<XMLElement*>;
	using NotifyFunc = std::function<void(XMLElement*)>;

	public:
		XMLParser () =default;
		virtual ~XMLParser() {}
		explicit XMLParser (XMLElement *root) {setRootElement(root);}
		XMLElement* setRootElement (XMLElement *root);
		void parse (std::istream &is);
		void parse (std::string xml, bool finish=false);
		void finish ();
		void setNotifyFuncs (NotifyFunc notifyElementOpened, NotifyFunc notifyElementClosed);

	protected:
		XMLElement* context () {return _elementStack.back();}
		virtual void appendNode (std::unique_ptr<XMLNode> node);
		virtual XMLElement* finishPushContext (std::unique_ptr<XMLElement> elem);
		virtual void finishPopContext () {}
		virtual XMLElement* openElement (const std::string &tag);
		virtual void closeElement (const std::string &tag);
		virtual XMLElement* createElementPtr (std::string name) const;

	private:
		std::string _xmlbuf;
		std::unique_ptr<XMLElement> _root;  ///< element holding the parsed nodes
		ElementStack _elementStack;         ///< elements not yet closed
		bool _error=false;
		std::function<void(XMLElement*)> _notifyElementOpened;
		std::function<void(XMLElement*)> _notifyElementClosed;
};

#endif
