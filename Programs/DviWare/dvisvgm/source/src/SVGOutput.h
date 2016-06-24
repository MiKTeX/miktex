/*************************************************************************
** SVGOutput.h                                                          **
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

#ifndef DVISVGM_SVGOUTPUT_H
#define DVISVGM_SVGOUTPUT_H

#include <ostream>
#include <string>
#include "FilePath.h"


struct SVGOutputBase {
	virtual ~SVGOutputBase () {}
	virtual std::ostream& getPageStream (int page, int numPages) const =0;
	virtual std::string filename (int page, int numPages) const =0;
};


class SVGOutput : public SVGOutputBase
{
	public:
		SVGOutput (const char *base=0, std::string pattern="", int zipLevel=0);
		~SVGOutput () {delete _os;}
		std::ostream& getPageStream (int page, int numPages) const;
		std::string filename (int page, int numPages) const;

	protected:
		void expandFormatString (std::string &str, int page, int numPages) const;

	private:
		FilePath _path;
		std::string _pattern;
		bool _stdout;      ///< write to STDOUT?
		int _zipLevel;     ///< compression level
		mutable int _page; ///< number of current page being written
		mutable std::ostream *_os;
};

#endif
