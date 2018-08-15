/* 1-3.cpp:

   Copyright (C) 2018 Christian Schenk

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

#include <miktex/Core/Test>

#include <string>

using namespace MiKTeX::Test;
using namespace std;

BEGIN_TEST_SCRIPT("process-1-3");

BEGIN_TEST_FUNCTION(1);
{
  throw TestException("process-1-3", "{a}errorMessage{b}", "{a}description{b}", "{a}remedy{b}", "tag", { "a", "x", "b", "y" }, MIKTEX_SOURCE_LOCATION());
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
