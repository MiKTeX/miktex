/*************************************************************************
** PageSize.hpp                                                         **
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

#ifndef PAGESIZE_HPP
#define PAGESIZE_HPP

#include "Length.hpp"
#include "MessageException.hpp"

struct PageSizeException : public MessageException {
	explicit PageSizeException (const std::string &msg) : MessageException(msg) {}
};

class PageSize {
	public:
		PageSize () =default;
		PageSize (Length w, Length h) : _width(w), _height(h) {}
		explicit PageSize (const std::string &name);
		void resize (std:: string name);
		void resize (Length w, Length h);
		Length width () const  {return _width;}
		Length height () const {return _height;}
		bool valid () const    {return _width.pt() > 0 && _height.pt() > 0;}

	private:
		Length _width, _height;
};

#endif
