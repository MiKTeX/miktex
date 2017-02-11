/* 1.cpp:

   Copyright (C) 1996-2017 Christian Schenk

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

#include <miktex/Core/Test>

#include <cstring>

#include <functional>

#include <miktex/Core/ci_string>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;
using namespace std;

BEGIN_TEST_SCRIPT("strings-1");

BEGIN_TEST_FUNCTION(1);
{
  ci_string s("AbCdE");
  TEST(s == "abcde");
  TEST(s == "ABCDE");
  TEST(strcmp(s.c_str(), "AbCdE") == 0);
  TEST(strcmp(s.c_str(), "abcde") != 0);
  hash<ci_string> hashfn;
  TEST(hashfn(s) == hashfn("abcde"));
  TEST(hashfn(s) == hashfn("ABCDE"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  TEST(StringUtil::UTF16ToUTF8(u"\u263A") == u8"\u263A");
  TEST(StringUtil::UTF8ToUTF16(u8"\u263A") == u"\u263A");
  TEST(StringUtil::UTF32ToUTF8(U"\u263A") == u8"\u263A");
  TEST(StringUtil::UTF8ToUTF32(u8"\u263A") == U"\u263A");
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
  CALL_TEST_FUNCTION(2);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
