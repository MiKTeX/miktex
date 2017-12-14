/* StdAfx.h: pre-compiled header stuff                  -*- C++ -*-

   Copyright (C) 2000-2017 Christian Schenk

   This file is part of MiKTeX Options.

   MiKTeX Options is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Options is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Options; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#pragma once

#if !defined(_SECURE_ATL)
#  define _SECURE_ATL 1
#endif

#if !defined(VC_EXTRALEAN)
#  define VC_EXTRALEAN
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>
#include <afxdtctl.h>
#include <afxmt.h>
#include <afxtempl.h>

#include <HtmlHelp.h>

#include <shlwapi.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
#  include <afxcmn.h>
#endif

#include "mo-version.h"

#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/Debug>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
#include <miktex/Core/win/winAutoResource>
#include <miktex/Core/win/WindowsVersion>
#include <miktex/Util/StringUtil>

#include <memory>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

#include <miktex/UI/MFC/ErrorDialog>
#include <miktex/UI/MFC/PackageTreeCtrl>
#include <miktex/UI/MFC/ProgressDialog>
#include <miktex/UI/MFC/SiteWizSheet>
#include <miktex/UI/MFC/TextViewerDialog>
#include <miktex/UI/MFC/UpdateDialog>

using namespace MiKTeX;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace MiKTeX::Util;
using namespace std;
using namespace std::string_literals;
