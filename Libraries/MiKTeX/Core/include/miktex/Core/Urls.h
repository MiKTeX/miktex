/* miktex/Core/urls.h: hard-coded urls                  -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(BCD264408F6CC94AB371F96954CBDA98)
#define BCD264408F6CC94AB371F96954CBDA98

#include <miktex/Version>

#define MIKTEX_URL_WWW_NO_SLASH "http://miktex.org"

#define MIKTEX_URL_WWW MIKTEX_URL_WWW_NO_SLASH "/"

#define MIKTEX_URL_WWW_GIVE_BACK MIKTEX_URL_WWW "giveback"

#define MIKTEX_URL_WWW_PACKAGING MIKTEX_URL_WWW "pkg/about"

#define MIKTEX_URL_WWW_PACKAGE_INFO_FORMAT MIKTEX_URL_WWW "packages/%s"

#if 1
#  define MIKTEX_URL_WWW_REGISTRATION MIKTEX_URL_WWW_GIVE_BACK
#else
#  define MIKTEX_URL_WWW_REGISTRATION MIKTEX_URL_WWW "registration"
#endif

#define MIKTEX_URL_WWW_KNOWN_ISSUES MIKTEX_URL_WWW MIKTEX_SERIES_STR "/issues"

#define MIKTEX_URL_WWW_SUPPORT MIKTEX_URL_WWW "support"

#endif
