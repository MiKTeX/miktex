/* 1.cpp:

   Copyright (C) 1996-2021 Christian Schenk

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

#include <memory>
#include <string>

#include <miktex/Core/Cfg>
#include <miktex/Core/StreamWriter>

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;

BEGIN_TEST_SCRIPT("cfg-1");

BEGIN_TEST_FUNCTION(1);
{
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->PutValue("ABC", "xYz", "abrakadabraa"));
  TESTX(cfg->PutValue("AbC", "xyZ", "abrakadabra"));
  TEST(cfg->GetValue("abc", "xyz")->AsString() == "abrakadabra");
  TESTX(cfg->PutValue("abc", "arr[]", "abc"));
  TESTX(cfg->PutValue("abc", "arr[]", "def"));
  vector<string> arr;
  TEST(cfg->TryGetValueAsStringVector("abc", "arr[]", arr));
  TEST(arr.size() == 2 && arr[0] == "abc" && arr[1] == "def");
  TESTX(cfg->PutValue("abc", "empty", ""));
  TESTX(cfg->Write(PathName("test.ini")));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->Read(PathName("test.ini")));
  TEST(cfg->GetValue("abc", "xyz")->AsString() == "abrakadabra");
  TESTX(cfg->DeleteValue("abc", "xyz"));
  string value;
  TEST(!cfg->TryGetValueAsString("abc", "xyz", value));
  vector<string> arr;
  TEST(cfg->TryGetValueAsStringVector("abc", "arr[]", arr));
  TEST(arr.size() == 2 && arr[0] == "abc" && arr[1] == "def");
  TEST(cfg->GetValue("abc", "empty")->AsString() == "");
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
  StreamWriter writer(PathName("test2.ini"));
  writer.WriteLine("[sec1]");
  writer.WriteLine("arr1[]= abc ");
  writer.WriteLine("arr1[]=     ");
  writer.WriteLine("arr2[]=abc");
  writer.WriteLine("!clear arr2[]");
  writer.WriteLine("!clear arr3[]");
  writer.Close();
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->Read(PathName("test2.ini")));
  vector<string> arr1;
  TEST(cfg->TryGetValueAsStringVector("sec1", "arr1[]", arr1));
  TEST(arr1.size() == 2 && arr1[0] == "abc" && arr1[1] == "");
  vector<string> arr2;
  TEST(cfg->TryGetValueAsStringVector("sec1", "arr2[]", arr2));
  TEST(arr2.size() == 0);
  vector<string> arr3;
  TEST(!cfg->TryGetValueAsStringVector("sec1", "arr3[]", arr3));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(4);
{
#if 0
  // TODO: can be enabled when we switch to EMSA3(SHA-256)
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->Read(MiKTeX::Util::PathName(TEST_SOURCE_DIR) / "cfg" / "sigtest.ini"));
#endif
}
END_TEST_FUNCTION();

class PrivateKeyProvider : public IPrivateKeyProvider
{
public:
  PathName MIKTEXTHISCALL GetPrivateKeyFile() override
  {
    return MiKTeX::Util::PathName(TEST_SOURCE_DIR) / PathName("cfg") / PathName("test.pkcs8.pem");
  }
public:
  bool GetPassphrase(std::string & passphrase) override
  {
    passphrase = "fortestingonly";
    return true;
  }
};

BEGIN_TEST_FUNCTION(5);
{
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->Read(PathName("test.ini")));
  PrivateKeyProvider privKey;
  TESTX(cfg->Write(PathName("sigtest2.ini"), "signed with test key", &privKey));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(6);
{
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  TESTX(cfg->Read(PathName("sigtest2.ini"), PathName(TEST_SOURCE_DIR) / PathName("cfg") / PathName("test.pub.pem")));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(7);
{
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  istringstream reader("[sec1]\nfoo=bar\n[sec2]\nfoo=bar\n");
  TESTX(cfg->Read(reader));
  TEST(cfg->GetValue("sec2", "foo")->AsString() == "bar");
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(8);
{
  shared_ptr<Cfg> cfg;
  TESTX(cfg = Cfg::Create());
  cfg->PutValue("abc", "value", "abra");
  cfg->PutValue("def", "value", "kadabra");
  vector<string> keys;
  vector<string> values;
  for (auto key : *cfg)
  {
    keys.push_back(key->GetName());
    for (auto val : *key)
    {
      values.push_back(val->AsString());
    }
  }
  TEST(keys.size() == 2);
  std::sort(keys.begin(), keys.end());
  TEST(keys[0] == "abc");
  TEST(keys[1] == "def");
  TEST(values.size() == 2);
  std::sort(values.begin(), values.end());
  TEST(values[0] == "abra");
  TEST(values[1] == "kadabra");
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
  CALL_TEST_FUNCTION(2);
  CALL_TEST_FUNCTION(3);
  CALL_TEST_FUNCTION(4);
  CALL_TEST_FUNCTION(5);
  CALL_TEST_FUNCTION(6);
  CALL_TEST_FUNCTION(7);
  CALL_TEST_FUNCTION(8);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
