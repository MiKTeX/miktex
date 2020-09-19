/* StdAfx.h: includes precompiled header files          -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

   This file is part of Yap.

   Yap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Yap is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Yap; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include <miktex/First>

#if !defined(_SECURE_ATL)
#define _SECURE_ATL 1
#endif

#if !defined(VC_EXTRALEAN)
//#  define VC_EXTRALEAN
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxmt.h>

#if ! defined(_AFX_NO_OLE_SUPPORT)
#  include <afxdtctl.h>
#endif

#if ! defined(_AFX_NO_AFXCMN_SUPPORT)
#  include <afxcmn.h>
#endif

#include <winspool.h>
#include <shlwapi.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <algorithm>
#include <stack>
#include <map>
#include <memory>
#include <vector>
#include <string>

#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <malloc.h>
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <sys/stat.h>
#include <shellapi.h>

#include <htmlhelp.h>

#include <ddeml.h>

#include "yap-version.h"

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Debug>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Core/win/winAutoResource>
#include <miktex/Core/win/HResult>
#include <miktex/DVI/Dvi>
#include <miktex/Graphics/DibChunker>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Wrappers/PoptWrapper>

#include <afxdlgs.h>

using namespace MiKTeX::Core;
using namespace MiKTeX::DVI;
using namespace MiKTeX::Graphics;
using namespace MiKTeX::Util;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Wrappers;
using namespace std;
using namespace std::string_literals;
