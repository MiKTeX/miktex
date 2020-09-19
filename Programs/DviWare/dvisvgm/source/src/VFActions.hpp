/*************************************************************************
** VFActions.hpp                                                        **
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

#ifndef VFACTIONS_HPP
#define VFACTIONS_HPP

#include <string>
#include <vector>


struct VFActions
{
	virtual ~VFActions () =default;
	virtual void vfPreamble (const std::string &comment, uint32_t checksum, double dsize) {}
	virtual void vfPostamble () {}
	virtual void defineVFFont (uint32_t fontnum, const std::string &path, const std::string &name, uint32_t checksum, double dsize, double ssize) {}
	virtual void defineVFChar (uint32_t c, std::vector<uint8_t> &&dvi) {}
};

#endif
