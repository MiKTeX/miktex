/* session.h: MiKTeX session                            -*- C++ -*-

   Copyright (C) 2006-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "core-version.h"

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
namespace MiKTeXSessionLib = MAKE_CURVER_ID(MiKTeXSession);
#endif

class MiKTeXSessionModule :
  public ATL::CAtlDllModuleT<MiKTeXSessionModule>
{
public:
  DECLARE_LIBID(__uuidof(MiKTeXSessionLib::MAKE_CURVER_ID(__MiKTeXSession)));

public:
  DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MIKTEXSESSIONMODULE, "{C3CAB8AD-25D8-4EB0-912A-084D08B956CE}");
};

extern MiKTeXSessionModule _AtlModule;
