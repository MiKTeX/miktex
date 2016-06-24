/* StdAfx.h: includes precompiled header files          -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

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

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <winspool.h>
#include <shlwapi.h>

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cmath>

#include <algorithm>
#include <stack>
#include <map>
#include <memory>
#include <vector>
#include <string>

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
#include <miktex/Core/CSVList>
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
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
#include <miktex/Core/win/winAutoResource>
#include <miktex/Core/win/DllProc>
#include <miktex/Core/win/HResult>
#include <miktex/DVI/Dvi>
#include <miktex/Graphics/DibChunker>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/UI/MFC/ErrorDialog>
#include <miktex/UI/MFC/ProgressDialog>
#include <miktex/UI/MFC/TextViewerDialog>
#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Wrappers/PoptWrapper>

#include <afxdlgs.h>

using namespace MiKTeX::Core;
using namespace MiKTeX::DVI;
using namespace MiKTeX::Graphics;
using namespace MiKTeX::Util;
using namespace MiKTeX::Trace;
using namespace MiKTeX::UI::MFC;
using namespace MiKTeX::Wrappers;
using namespace std;
