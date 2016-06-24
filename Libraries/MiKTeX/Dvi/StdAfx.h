/* StdAfx.h:                                            -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#pragma once

#include <miktex/First>

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <cctype>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <stack>
#include <string>
#include <unordered_map>

#include <miktex/Core/AutoResource>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Debug>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/TemporaryFile>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/winAutoResource>
#endif

#include <miktex/Graphics/DibChunker>

#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include <miktex/Util/StringUtil>
