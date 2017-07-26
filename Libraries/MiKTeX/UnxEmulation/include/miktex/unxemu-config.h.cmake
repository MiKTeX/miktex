/* miktex/unxemu-config.h:                             -*- C++ -*-

   Copyright (C) 2008-2017 Christian Schenk

   This file is part of the MiKTeX UNXEMU Library.

   The MiKTeX UNXEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX UNXEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX UNXEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

/* This file was generated from miktex/unxemu-config.h.cmake. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(D5D965B9944A4F40B5E53EDA36F6FC72)
#define D5D965B9944A4F40B5E53EDA36F6FC72

#include <miktex/Definitions.h>

// DLL import/export switch
#if !defined(D2A2BA842ACE40C6A8A17A9358F2147E)
#  define MIKTEXUNXEXPORT MIKTEXDLLIMPORT
#endif

// API decoration for exported functions
#define MIKTEXUNXCEEAPI(type) MIKTEXUNXEXPORT type MIKTEXCEECALL

#cmakedefine HAVE_ACCESS 1
#cmakedefine HAVE_ALLOCA 1
#cmakedefine HAVE_CHDIR 1
#cmakedefine HAVE_CHMOD 1
#cmakedefine HAVE_FINITE 1
#cmakedefine HAVE_GETCWD 1
#cmakedefine HAVE_GETPID 1
#cmakedefine HAVE_MKDIR 1
#cmakedefine HAVE_MKSTEMP 1
#cmakedefine HAVE_OPEN 1
#cmakedefine HAVE_PCLOSE 1
#cmakedefine HAVE_POPEN 1
#cmakedefine HAVE_RENAME 1
#cmakedefine HAVE_RMDIR 1
#cmakedefine HAVE_STAT 1
#cmakedefine HAVE_UNLINK 1
#cmakedefine HAVE_UTIME 1

#endif
