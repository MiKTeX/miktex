/*************************************************************************
** PSPreviewHandler.hpp                                                 **
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

#ifndef PSPREVIEWHANDLER_HPP
#define PSPREVIEWHANDLER_HPP

#include <string>
#include <vector>
#include "BoundingBox.hpp"

class PSInterpreter;

class PSPreviewHandler {
	public:
		explicit PSPreviewHandler (PSInterpreter &psi);
		void init ();
		std::string version () const  {return _version;}
		bool tightpage () const       {return _tightpage;}
		bool readDataFromStack ();
		void setDviScaleFactor (double dvi2bp) {_dvi2bp = dvi2bp;}
		bool getBoundingBox (BoundingBox &bbox) const;
		double height () const;
		double depth () const;
		double width () const;

	private:
		std::string _version;         ///< version string of preview package
		bool _tightpage=false;        ///< true if tightpage option was given
		double _dvi2bp=1.0/65536.0;   ///< factor to convert dvi units to PS points
		std::vector<int> _boxExtents; ///< bounding box data set by the preview package (in DVI units)
		PSInterpreter &_psi;
};

#endif
