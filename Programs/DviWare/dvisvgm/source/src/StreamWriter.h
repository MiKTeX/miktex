/*************************************************************************
** StreamWriter.h                                                       **
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

#ifndef DVISVGM_STREAMWRITER_H
#define DVISVGM_STREAMWRITER_H

#include <ostream>
#include "types.h"


class CRC32;

class StreamWriter
{
	public:
		StreamWriter (std::ostream &os) : _os(os) {}
		virtual ~StreamWriter () {}
		void writeUnsigned (UInt32 val, int n);
		void writeSigned (Int32 val, int n);
		void writeString (const std::string &str, bool finalZero=false);
		void writeUnsigned (UInt32 val, int n, CRC32 &crc32);
		void writeSigned (Int32 val, int n, CRC32 &crc32);
		void writeString (const std::string &str, CRC32 &crc32, bool finalZero=false);

	private:
		std::ostream &_os;
};

#endif
