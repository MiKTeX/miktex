/*************************************************************************
** MetafontWrapper.hpp                                                  **
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

#ifndef METAFONTWRAPPER_HPP
#define METAFONTWRAPPER_HPP

#include <string>


class FileFinder;

class MetafontWrapper {
	public:
		MetafontWrapper (std::string fname, std::string dir);
		bool call (const std::string &mode, double mag);
		bool make (const std::string &mode, double mag);
		bool success () const;

	protected:
		int getResolution (const std::string &mfMessage) const;

	private:
		std::string _fontname;
		std::string _dir;
};

#endif
