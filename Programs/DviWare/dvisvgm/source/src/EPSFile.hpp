/*************************************************************************
** EPSFile.hpp                                                          **
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

#ifndef EPSFILE_HPP
#define EPSFILE_HPP

#include <fstream>
#include <istream>
#include <string>
#include "BoundingBox.hpp"

class EPSFile {
	public:
		explicit EPSFile (const std::string &fname);
		std::istream& istream () const;
		bool hasValidHeader () const {return _headerValid;}
		BoundingBox bbox () const;
		uint32_t pslength () const {return _pslength;}

	private:
		mutable std::ifstream _ifs;
		bool _headerValid=false; ///< true if file has a valid header
		uint32_t _offset=0;      ///< stream offset where ASCII part of the file begins
		uint32_t _pslength=0;    ///< length of PS section (in bytes)
};

#endif
