/* config.h (created from config.h.cmake)               -*- C++ -*-

   Copyright (C) 2001-2018 Christian Schenk

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

#if defined(MIKTEX_MPM_SHARED)
#  define MIKTEXMPMEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXMPMEXPORT
#endif

#define F927BA187CB94546AB9CA9099D989E81
#include "miktex/PackageManager/config.h"

#define BEGIN_INTERNAL_NAMESPACE                        \
namespace MiKTeX {                                      \
  namespace Packages {                                  \
    namespace D6AAD62216146D44B580E92711724B78 {

#define END_INTERNAL_NAMESPACE                  \
    }                                           \
  }                                             \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

#define IGNORE_OTHER_SYSTEMS 1

#if defined(MIKTEX_WINDOWS)
#define ASYNC_LAUNCH_POLICY (std::launch::deferred | std::launch::async)
#else
#define ASYNC_LAUNCH_POLICY std::launch::deferred
#endif

#define MPMSTATICFUNC(type) static type
#define MPMINTERNALFUNC(type) type
#define MPMINTERNALVAR(type) type

#define ZZDB1_SIZE 1024 * 40
#define ZZDB2_SIZE 1024 * 245

#define DB_ARCHIVE_FILE_TYPE ArchiveFileType::TarLzma

#include "mpm-version.h"
