/* COMInitializer.cpp:

   Copyright (C) 2021 Christian Schenk

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

#include "config.h"

#include <miktex/Core/win/COMInitializer>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;

COMInitializer::COMInitializer()
{
  hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (hr.Failed() && hr != RPC_E_CHANGED_MODE)
  {
    MIKTEX_FATAL_ERROR_2(T_("COM library could not be initialized."), "hr", hr.ToString());
  }
}

COMInitializer::COMInitializer(DWORD dwCoInit)
{
  hr = CoInitializeEx(nullptr, dwCoInit);
  if (hr.Failed())
  {
    MIKTEX_FATAL_ERROR_2(T_("COM library could not be initialized."), "hr", hr.ToString());
  }
}

COMInitializer::~COMInitializer()
{
  if (hr.Succeeded())
  {
    CoUninitialize();
  }
}
