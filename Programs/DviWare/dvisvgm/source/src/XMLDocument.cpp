/*************************************************************************
** XMLDocument.cpp                                                      **
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

#include "XMLDocument.hpp"

using namespace std;

XMLDocument::XMLDocument (unique_ptr<XMLElement> root)
	: _rootElement(std::move(root))
{
}


void XMLDocument::clear () {
	_rootElement.reset();
	_nodes.clear();
}


void XMLDocument::append (unique_ptr<XMLNode> node) {
	if (node) {
		if (node->toElement())
			_rootElement = util::static_unique_ptr_cast<XMLElement>(std::move(node));
		else
			_nodes.emplace_back(std::move(node));
	}
}


void XMLDocument::setRootNode (unique_ptr<XMLElement> root) {
	_rootElement = std::move(root);
}


ostream& XMLDocument::write (ostream &os) const {
	if (_rootElement) { // no root element => no output
		os << "<?xml version='1.0' encoding='UTF-8'?>\n";
		for (const auto &node : _nodes) {
			node->write(os);
			os << '\n';
		}
		_rootElement->write(os);
	}
	return os;
}
