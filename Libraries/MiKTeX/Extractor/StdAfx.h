/* StdAfx.h: pre-compiled header stuff                  -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MiKTeX Extractor.

   MiKTeX Extractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.
   
   MiKTeX Extractor is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with MiKTeX Extractor; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
#  include <Fdi.h>
#endif

#include <algorithm>
#include <locale>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include <mspack.h>

#if defined(_MSC_VER)
#  include <malloc.h>
#  include <direct.h>
#  include <io.h>
#  include <share.h>
#endif

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <cstdarg>
#include <cstddef>

#include <miktex/Core/BZip2Stream>
#include <miktex/Core/Debug>
#include <miktex/Core/Directory>
#include <miktex/Core/Environment>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/FileType>
#include <miktex/Core/LzmaStream>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Quoter>
#include <miktex/Core/Registry>
#include <miktex/Core/Text>

#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include <miktex/Util/inliners.h>
