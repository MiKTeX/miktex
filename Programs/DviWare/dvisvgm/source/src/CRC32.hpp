/*************************************************************************
** CRC32.hpp                                                            **
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

#ifndef CRC32_HPP
#define CRC32_HPP

#include <cstdlib>
#include <istream>

class CRC32 {
	public:
		CRC32 ();
		CRC32 (const CRC32 &crc32) =delete;
		void update (const uint8_t *bytes, size_t len);
		void update (uint32_t n, int bytes=4);
		void update (const char *str);
		void update (std::istream &is);
		uint32_t get () const;
		void reset ();
		static uint32_t compute (const uint8_t *bytes, size_t len);
		static uint32_t compute (const char *str);
		static uint32_t compute (std::istream &is);

	private:
		uint32_t _crc32;
		uint32_t _tab[256];
};


#endif
