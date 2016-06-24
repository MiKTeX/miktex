/* StdAfx.h: pre-compiled header stuff                  -*- C++ -*-

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

#pragma once

#include <cstdlib>
#if defined(MIKTEX_UNIX)
#include <sys/time.h>
#endif

#include <iostream>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Directory>
#include <miktex/Core/Registry>
#include <miktex/Util/StringUtil>
#include <miktex/Version>

#include <string>

using namespace MiKTeX::Core;
