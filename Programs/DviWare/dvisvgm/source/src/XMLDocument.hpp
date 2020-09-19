/*************************************************************************
** XMLDocument.hpp                                                      **
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

#ifndef XMLDOCUMENT_HPP
#define XMLDOCUMENT_HPP

#include <memory>
#include <vector>
#include "XMLNode.hpp"

class XMLDocument {
	public:
		XMLDocument () =default;
		explicit XMLDocument(std::unique_ptr<XMLElement> root);
		void clear ();
		void append (std::unique_ptr<XMLElement> node);
		void append (std::unique_ptr<XMLNode> node);
		void setRootNode (std::unique_ptr<XMLElement> root);
		const XMLElement* getRootElement () const {return _rootElement.get();}
		std::ostream& write (std::ostream &os) const;

	private:
		std::vector<std::unique_ptr<XMLNode>> _nodes;
		std::unique_ptr<XMLElement> _rootElement;
};

#endif
