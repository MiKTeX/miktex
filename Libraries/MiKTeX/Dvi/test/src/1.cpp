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

#include "miktextest.h"
#include <miktex/DVI/Dvi>

BEGIN_TEST_FUNC(1);
{
  try
    {
      MiKTeX::Core::TraceStream::SetTraceFlags("dvisearch,error,DviPage");
      Dvi * pDvi =
	Dvi::Create("dvidir/test.dvi", "ljfour", 600, 5, DviAccess::Random);
      TEST (pDvi != 0);
      int p, x, y;
      TEST (pDvi->FindSource("test.tex", 1234, p, x, y));
      TEST (p == 16);
      string strFQ = GetCWD() + "/dvidir/test.tex";
      TEST (pDvi->FindSource(strFQ.c_str(), 1234, p, x, y));
      delete pDvi;
    }
  catch (const e_general &)
    {
      TEST (false);
    }
}
END_TEST_FUNC();

BEGIN_TEST_FUNC(2);
{
  try
    {
      MiKTeX::Core::TraceStream::SetTraceFlags("dvisearch,error,DviPage");
      Dvi * pDvi =
	Dvi::Create("./test-rel.dvi", "ljfour", 600, 5, DviAccess::Random);
      TEST (pDvi != 0);
      int p, x, y;
      TEST (pDvi->FindSource("./dvidir/test.tex", 1234, p, x, y));
      TEST (pDvi->FindSource("dvidir/test.tex", 1234, p, x, y));
      string strFQ = GetCWD() + "/dvidir/test.tex";
      TEST (pDvi->FindSource(strFQ.c_str(), 1234, p, x, y));
      delete pDvi;
    }
  catch (const e_general &)
    {
      TEST (false);
    }
}
END_TEST_FUNC();

BEGIN_TEST_FUNC(3);
{
  try
    {
      MiKTeX::Core::TraceStream::SetTraceFlags("dvisearch,error,DviPage");
      Dvi * pDvi =
	Dvi::Create("dvidir/test-fq.dvi", "ljfour",
		    600, 5, DviAccess::Random);
      TEST (pDvi != 0);
      int p, x, y;
      TEST (pDvi->FindSource("test.tex", 1234, p, x, y));
      TEST (pDvi->FindSource("./test.tex", 1234, p, x, y));
      string strFQ = GetCWD() + "/dvidir/test.tex";
      TEST (pDvi->FindSource(strFQ.c_str(), 1234, p, x, y));
      delete pDvi;
    }
  catch (const e_general &)
    {
      TEST (false);
    }
}
END_TEST_FUNC();

BEGIN_TEST_PROGRAM();
{
  DOTESTS(1);
  DOTESTS(2);
  DOTESTS(3);
}
END_TEST_PROGRAM();
