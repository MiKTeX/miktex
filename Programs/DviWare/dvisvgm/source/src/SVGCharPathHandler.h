/*************************************************************************
** SVGCharPathHandler.h                                                 **
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

#ifndef SVGCHARPATHHANDLER_H
#define SVGCHARPATHHANDLER_H

#include "SVGCharHandler.h"

/** Creates SVG path elements for all characters. */
class SVGCharPathHandler : public SVGCharHandler {
	typedef void (SVGCharPathHandler::*AppendMethod)(UInt32 c, double x, double y, const Matrix &matrix);

	public:
		SVGCharPathHandler (bool createUseElements, bool relativePathsCommands);
		void appendChar (UInt32 c, double x, double y);

	protected:
		void appendUseElement (UInt32 c, double x, double y, const Matrix &matrix);
		void appendPathElement (UInt32 c, double x, double y, const Matrix &matrix);
		void resetContextNode ();

	private:
		AppendMethod _appendChar;       ///< method called to append a single character
		bool _relativePathCommands;     ///< if true, create relative rather than absolute SVG path commands
		XMLElementNode *_groupNode;     ///< current group node taking the path elements
		CharProperty<Color> _fontColor; ///< color of current font
};

#endif
