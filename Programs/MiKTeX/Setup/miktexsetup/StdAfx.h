/* StdAfx.h:                                            -*- C++ -*-

   Copyright (C) 2014-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include <climits>
#include <cstdio>

#include <stdarg.h>

#if defined(_MSC_VER)
#  pragma warning (push, 1)
#  pragma warning (disable: 4702)
#endif

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#if defined(_WIN32)
#  include <Windows.h>
#endif

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <signal.h>

#if defined(_MSC_VER)
#  pragma warning (pop)
#endif

#include "miktexsetup-version.h"

#include <miktex/Core/Exceptions>
#include <miktex/Core/Paths>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Setup/SetupService>
#include <miktex/Trace/Trace>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>

#if defined(MIKTEX_WINDOWS)
#  include <MiKTeX/Core/Help>
#endif

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;
