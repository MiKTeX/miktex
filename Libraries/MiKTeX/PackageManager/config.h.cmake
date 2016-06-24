/* config.h (created from config.h.cmake)		-*- C++ -*-

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#cmakedefine HAVE_ATLBASE_H 1
#cmakedefine HAVE_FLOAT_H 1
#cmakedefine HAVE_FTIME 1
#cmakedefine HAVE_GMTIME 1
#cmakedefine HAVE_HASH_MAP 1
#cmakedefine HAVE_HASH_SET 1
#cmakedefine HAVE_LIMITS_H 1
#cmakedefine HAVE_MATH_H 1
#cmakedefine HAVE_MBTOWC 1
#cmakedefine HAVE_SNPRINTF 1
#cmakedefine HAVE_SPRINTF_L 1
#cmakedefine HAVE_SSCANF 1
#cmakedefine HAVE_STRRCHR 1
#cmakedefine HAVE_STRTOD 1
#cmakedefine HAVE_STRTOD_L 1
#cmakedefine HAVE_STRTOL 1
#cmakedefine HAVE_STRTOLL 1
#cmakedefine HAVE_STRTOUL 1
#cmakedefine HAVE_STRTOULL 1
#cmakedefine HAVE_SYS_TIMEB_H 1
#cmakedefine HAVE_UNORDERED_MAP 1
#cmakedefine HAVE_UNORDERED_SET 1
#cmakedefine HAVE_WCTOMB 1

#if defined(_MSC_VER)
#  define SOAP_LONG_FORMAT "%I64d"
#  define SOAP_ULONG_FORMAT "%I64u"
#endif
