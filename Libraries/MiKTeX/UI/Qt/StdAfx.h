/* StdAfx.h: pre-compiled header stuff

   Copyright (C) 2008-2016 Christian Schenk

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

#include <QDateTime>
#include <QtWidgets>
#include <QUrl>

#if defined(MIKTEX_WINDOWS)
#  include <QtWinExtras/qwinfunctions.h>
#  include <Windows.h>
#  include <Commctrl.h>
#  include <shellapi.h>
#  define SECURITY_WIN32
#  include <Security.h>
#endif

#include <algorithm>
#include <sstream>
#include <string>

#include <miktex/Core/Debug>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/Paths>
#include <miktex/Core/Registry>
#include <miktex/Core/Session>

#include <miktex/PackageManager/PackageManager>

#if defined(MIKTEX_WINDOWS)
#  include <miktex/Core/win/winAutoResource>
#  include <miktex/Core/win/WindowsVersion>
#  include <miktex/Core/win/DllProc>
#endif
