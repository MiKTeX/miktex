/*************************************************************************
** types.h                                                              **
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

#ifndef DVISVGM_TYPES_H
#define DVISVGM_TYPES_H

namespace internal {
	template<unsigned BYTES>
	class ERROR_inttype_not_available
	{
		ERROR_inttype_not_available();
	};

	template<bool FIRST, typename A, typename B>
	struct select
	{
		typedef A T;
	};

	template<typename A, typename B>
	struct select<false, A, B>
	{
		typedef B T;
	};
}


// Retrieves a signed integer type with sizeof(T) == BYTES
template<unsigned BYTES, bool SIGNED>
struct int_t
{
   typedef typename internal::select<sizeof(signed char)      == BYTES, signed char,
           typename internal::select<sizeof(signed short)     == BYTES, signed short,
           typename internal::select<sizeof(signed int)       == BYTES, signed int,
           typename internal::select<sizeof(signed long)      == BYTES, signed long,
           typename internal::select<sizeof(signed long long) == BYTES, signed long long,
           internal::ERROR_inttype_not_available<BYTES> >::T>::T>::T>::T>::T T;
};


// Retrieves an unsigned integer type with sizeof(T) == BYTES
template<unsigned BYTES>
struct int_t<BYTES, false>
{
   typedef typename internal::select<sizeof(unsigned char)      == BYTES, unsigned char,
           typename internal::select<sizeof(unsigned short)     == BYTES, unsigned short,
           typename internal::select<sizeof(unsigned int)       == BYTES, unsigned int,
           typename internal::select<sizeof(unsigned long)      == BYTES, unsigned long,
           typename internal::select<sizeof(unsigned long long) == BYTES, unsigned long long,
           internal::ERROR_inttype_not_available<BYTES> >::T>::T>::T>::T>::T T;
};


// Machine independent definition of sized integer types
typedef int_t<1, true>::T  Int8;
typedef int_t<2, true>::T  Int16;
typedef int_t<4, true>::T  Int32;
typedef int_t<8, true>::T  Int64;
typedef int_t<1, false>::T UInt8;
typedef int_t<2, false>::T UInt16;
typedef int_t<4, false>::T UInt32;
typedef int_t<8, false>::T UInt64;

typedef Int32  FixWord;
typedef UInt32 ScaledInt;

#endif
