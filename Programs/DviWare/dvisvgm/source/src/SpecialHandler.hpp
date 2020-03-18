/*************************************************************************
** SpecialHandler.hpp                                                   **
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

#ifndef SPECIALHANDLER_HPP
#define SPECIALHANDLER_HPP

#include <istream>
#include <list>
#include <vector>
#include "MessageException.hpp"


class SpecialActions;
class SpecialManager;


struct SpecialException : public MessageException {
	explicit SpecialException (const std::string &msg) : MessageException(msg) {}
};


class SpecialHandler {
	friend class SpecialManager;
	public:
		virtual ~SpecialHandler () =default;
		virtual const char* info () const=0;
		virtual const char* name () const=0;
		virtual std::vector<const char*> prefixes() const =0;
		virtual void setDviScaleFactor (double dvi2bp) {}
		virtual void preprocess (const std::string &prefix, std::istream &is, SpecialActions &actions) {}
		virtual bool process (const std::string &prefix, std::istream &is, SpecialActions &actions)=0;
		virtual void dviPreprocessingFinished () {}
		virtual void dviBeginPage (unsigned pageno, SpecialActions &actions) {}
		virtual void dviEndPage (unsigned pageno, SpecialActions &actions) {}
		virtual void dviMovedTo (double x, double y, SpecialActions &actions) {}
};

#endif
