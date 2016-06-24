/* stdafx.h:                                            -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

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
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if ! defined(_SECURE_ATL)
#  define _SECURE_ATL 1
#endif

#if ! defined(VC_EXTRALEAN)
#  define VC_EXTRALEAN
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _AFX_ALL_WARNINGS

#include <afxcview.h>
#include <afxdlgs.h>
#include <afxdtctl.h>
#include <afxext.h>
#include <afxmt.h>
#include <afxwin.h>

#if ! defined(_AFX_NO_AFXCMN_SUPPORT)
#  include <afxcmn.h>
#endif

#if defined(_UNICODE)
#if defined(_M_IX86)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined(_M_IA64)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined(_M_X64)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include "mpm-version.h"

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Debug>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Session>
#include <miktex/Core/win/DllProc>
#include <miktex/Core/win/WindowsVersion>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/inliners.h>

#pragma warning (push, 1)
#pragma warning (disable: 4702)
#include <map>
#include <memory>
#include <string>
#include <vector>
#pragma warning (pop)

#include <miktex/UI/MFC/ErrorDialog>
#include <miktex/UI/MFC/ProgressDialog>
#include <miktex/UI/MFC/PropSheetPackage>
#include <miktex/UI/MFC/SiteWizSheet>
#include <miktex/UI/MFC/UpdateDialog>

using namespace MiKTeX;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::UI::MFC;
using namespace MiKTeX::Util;
using namespace std;

