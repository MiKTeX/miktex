/* StdAfx.h: pre-compiled header stuff

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the MiKTeX UI Library.

   The MiKTeX UI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if ! defined(VC_EXTRALEAN)
#  define VC_EXTRALEAN
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <afxinet.h>
#include <afxmt.h>
#include <afxtempl.h>

#include <Shlobj.h>

#include <algorithm>
#include <locale>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Debug>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/Paths>
#include <miktex/Core/Registry>
#include <miktex/Core/Urls>
#include <miktex/Core/win/winAutoResource>
#include <miktex/Core/win/WindowsVersion>
#include <miktex/PackageManager/PackageManager>
