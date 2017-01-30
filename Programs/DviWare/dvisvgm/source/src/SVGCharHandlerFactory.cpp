/*************************************************************************
** SVGCharHandlerFactory.cpp                                            **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include "SVGCharHandlerFactory.hpp"
#include "SVGCharPathHandler.hpp"
#include "SVGSingleCharTextHandler.hpp"
#include "SVGCharTspanTextHandler.hpp"
#include "SVGTree.hpp"


SVGCharHandler* SVGCharHandlerFactory::createHandler () {
	if (!SVGTree::USE_FONTS)
		return new SVGCharPathHandler(SVGTree::CREATE_USE_ELEMENTS, SVGTree::RELATIVE_PATH_CMDS);
	if (SVGTree::MERGE_CHARS)
		return new SVGCharTspanTextHandler(SVGTree::CREATE_CSS);
	else
		return new SVGSingleCharTextHandler(SVGTree::CREATE_CSS);
	return 0;
}