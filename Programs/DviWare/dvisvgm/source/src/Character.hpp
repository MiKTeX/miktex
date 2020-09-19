/*************************************************************************
** Character.hpp                                                        **
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

#ifndef CHARACTER_HPP
#define CHARACTER_HPP


class Character {
	public:
		enum Type {CHRCODE, INDEX, NAME};
		explicit Character (const char *name) : _type(NAME), _name(name) {}
		Character (Type type, uint32_t val) : _type(type), _number(val) {}
		Character (Type type, const Character &c) : _type(type), _number(c.type() != NAME ? c._number : 0) {}
		Type type () const {return _type;}
		const char* name () const {return _name;}
		uint32_t number () const  {return _number;}

	private:
		Type _type;
		union {
			uint32_t _number;
			const char *_name;
		};
};

#endif

