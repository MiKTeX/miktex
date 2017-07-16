/* StdAfx.h: pre-compiled header stuff                  -*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the MiKTeX KPSEMU Library.

   The MiKTeX KPSEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX KPSEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX KPSEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include <cstdarg>
#include <cstdlib>
#if defined(MIKTEX_UNIX)
#include <sys/time.h>
#endif

#include <miktex/App/Application>
#include <miktex/C4P/C4P>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Cfg>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Registry>
#include <miktex/TeXAndFriends/Prototypes>
#include <miktex/Util/CharBuffer>
#include <miktex/Version>

#include <string>
