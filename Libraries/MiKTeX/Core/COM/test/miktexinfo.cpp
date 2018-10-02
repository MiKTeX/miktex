/* miktexinfo.cpp:

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

#include "StdAfx.h"

using namespace std;

int main(int argc, char ** argv)
{
  if (FAILED(CoInitialize(nullptr)))
  {
    cerr << "COM library could not be initialized." << endl;
    return 1;
  }

  int retCode = 0;

  try
  {
    ISession2Ptr session;
    HRESULT hr = session.CreateInstance(L"MiKTeX.Session");
    if (FAILED(hr))
    {
      _com_raise_error(hr);
    }
    MiKTeXSetupInfo info = session->GetMiKTeXSetupInfo();
  }
  catch (const _com_error & e)
  {
    cerr << hex << e.Error() << endl;
    retCode = 1;
  }

  CoUninitialize();

  return retCode;
}
