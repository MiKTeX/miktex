/* 1.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX DVI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include <miktex/Core/Test>

#include <miktex/DVI/Dvi>

using namespace MiKTeX::DVI;

BEGIN_TEST_SCRIPT();

// simulate user
BEGIN_TEST_FUNCTION(1);
{
  auto_ptr<Dvi> pDvi (Dvi::Create(T_("test.dvi"),
				  T_("ljfour"),
				  600,
				  5,
				  DviAccess::Random,
				  0));
  for (int i = 0; i < pDvi->GetNumberOfPages() && i < 60; ++ i)
    {
      DviPage * pPage = pDvi->GetLoadedPage(i);
      TEST (pPage != 0);
      for (int j = 0; j < pPage->GetNumberOfBitmaps(); ++ j)
	{
	  const DviBitmap & bitmap = pPage->GetBitmap(j);
	}
      pPage->Unlock ();
      if (i % 3 == 0)
	{
	  Sleep (2000);
	}
    }
  pDvi->Dispose ();
  pDvi.reset ();
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION (1);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT ();
