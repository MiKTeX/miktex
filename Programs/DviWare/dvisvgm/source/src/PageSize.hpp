/*************************************************************************
** PageSize.hpp                                                         **
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

#ifndef PAGESIZE_HPP
#define PAGESIZE_HPP

#include "MessageException.hpp"

struct PageSizeException : public MessageException
{
	PageSizeException (const std::string &msg) : MessageException(msg) {}
};

class PageSize
{
	public:
		PageSize (double w=0, double h=0) : _width(w), _height(h) {}
		PageSize (const std::string &name);
		void resize (std:: string name);
		void resize (double w, double h);
		double widthInBP () const  {return _width;}
		double heightInBP () const {return _height;}
		double widthInMM () const  {return _width*25.4/72;}
		double heightInMM () const {return _height*25.4/72;}
		bool valid () const        {return _width > 0 && _height > 0;}

	private:
		double _width, _height; // in PS points
};

#endif
