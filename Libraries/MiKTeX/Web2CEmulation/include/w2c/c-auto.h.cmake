/* w2c/c-auto.h: Web2C emulation                        -*- C++ -*-

   Copyright (C) 2010-2016 Christian Schenk

   This file is part of the MiKTeX W2CEMU Library.

   The MiKTeX W2CEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX W2CEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX W2CEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(F9E93C82D7E8486A97E636B80AC78CE0)
#define F9E93C82D7E8486A97E636B80AC78CE0

#include <miktex/W2C/pre.h>
#include <miktex/Version.h>

#cmakedefine HAVE_DIRENT_H 1
#cmakedefine HAVE_MKSTEMP 1
#cmakedefine HAVE_MKTEMP 1
#cmakedefine HAVE_STRUCT_STAT_ST_MTIM 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_SYS_WAIT_H 1
#cmakedefine HAVE_UNISTD_H 1

#define WEB2CVERSION " (" MIKTEX_BANNER_STR ")"

#endif
