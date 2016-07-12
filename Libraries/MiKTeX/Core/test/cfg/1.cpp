/* 1.cpp:

   Copyright (C) 1996-2016 Christian Schenk

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

#include <memory>
#include <string>

#include <miktex/Core/Cfg>
#include <miktex/Core/StreamWriter>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

BEGIN_TEST_SCRIPT("cfg-1");

BEGIN_TEST_FUNCTION(1);
{
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->PutValue("ABC", "xYz", "abrakadabraa"));
  TESTX(cfg->PutValue("AbC", "xyZ", "abrakadabra"));
  TEST(cfg->GetValue("abc", "xyz") == "abrakadabra");
  TESTX(cfg->PutValue("abc", "arr[]", "abc"));
  TESTX(cfg->PutValue("abc", "arr[]", "def"));
  vector<string> arr;
  TEST(cfg->TryGetValue("abc", "arr[]", arr));
  TEST(arr.size() == 2 && arr[0] == "abc" && arr[1] == "def");
  TESTX(cfg->PutValue("abc", "empty", ""));
  TESTX(cfg->Write("test.ini"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->Read("test.ini"));
  TEST(cfg->GetValue("abc", "xyz") == "abrakadabra");
  TESTX(cfg->DeleteValue("abc", "xyz"));
  string value;
  TEST(!cfg->TryGetValue("abc", "xyz", value));
  vector<string> arr;
  TEST(cfg->TryGetValue("abc", "arr[]", arr));
  TEST(arr.size() == 2 && arr[0] == "abc" && arr[1] == "def");
  TEST(cfg->GetValue("abc", "empty") == "");
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
  StreamWriter writer("test2.ini");
  writer.WriteLine("[sec1]");
  writer.WriteLine("arr1[]= abc ");
  writer.WriteLine("arr1[]=     ");
  writer.WriteLine("arr2[]=abc");
  writer.WriteLine("!clear arr2[]");
  writer.WriteLine("!clear arr3[]");
  writer.Close();
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->Read("test2.ini"));
  vector<string> arr1;
  TEST(cfg->TryGetValue("sec1", "arr1[]", arr1));
  TEST(arr1.size() == 2 && arr1[0] == "abc" && arr1[1] == "");
  vector<string> arr2;
  TEST(cfg->TryGetValue("sec1", "arr2[]", arr2));
  TEST(arr2.size() == 0);
  vector<string> arr3;
  TEST(!cfg->TryGetValue("sec1", "arr3[]", arr3));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
  CALL_TEST_FUNCTION(2);
  CALL_TEST_FUNCTION(3);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
