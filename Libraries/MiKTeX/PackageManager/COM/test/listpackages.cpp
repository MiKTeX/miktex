/* listpackages.cpp:

   Copyright (C) 2001-2016 Christian Schenk

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
    IPackageManager2Ptr packageManager;
    HRESULT hr = packageManager.CreateInstance(L"MiKTeX.PackageManager");
    if (FAILED(hr))
    {
      _com_raise_error(hr);
    }
    IPackageIterator2Ptr packageIterator2 = packageManager->CreatePackageIterator();
    PackageInfo2 info;
    while (packageIterator2->GetNextPackageInfo2(&info))
    {
      wcout << info.displayName << endl;
    }
  }
  catch (const _com_error & e)
  {
    cerr << hex << e.Error() << endl;
    retCode = 1;
  }

  CoUninitialize();

  return retCode;
}
