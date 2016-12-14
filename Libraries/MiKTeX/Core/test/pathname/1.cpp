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

#include <miktex/Core/PathName>
#include <miktex/Core/PathNameParser>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

BEGIN_TEST_SCRIPT("pathname-1");

BEGIN_TEST_FUNCTION(1);
{
  TEST(PathName::Compare("//aBc/[e]/ghi.jkl", "//aBc/[e]/qwe.rty", 10) == 0);
  TEST(PathName::Compare("/abc/def/ghi.jkl", "/abc/def/qwe.rty", 10) != 0);
  PathName path("/abc/def/ghi.jkl");
  TEST(PathName::Compare(path.GetDirectoryName(), "/abc/def") == 0);
  TEST(PathName::Compare(path.GetDirectoryName(), "/abc/def/") == 0);
  TEST(PathName::Compare(path.GetFileNameWithoutExtension(), "ghi") == 0);
  TEST(PathName::Compare(path.GetExtension(), ".jkl") == 0);
  TEST(PathName::Compare(path.GetExtension(), "jkl") != 0);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
#if defined(MIKTEX_WINDOWS)
  PathNameParser component("C:/abc/def/ghi.jkl");
  TEST(PathName::Compare(*component, "C:/abc") == 0);
  ++component;
  TEST(PathName::Compare(*component, "def") == 0);
  ++component;
  TEST(PathName::Compare(*component, "ghi.jkl") == 0);
  ++component;
  TEST(!component);
#endif
  PathNameParser component2("/abc/def/ghi.jkl");
  TEST(PathName::Compare(*component2, "/abc") == 0);
  ++component2;
  TEST(PathName::Compare(*component2, "def") == 0);
  ++component2;
  TEST(PathName::Compare(*component2, "ghi.jkl") == 0);
  ++component2;
  TEST(!component2);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
  PathNameParser component("//abc/def/ghi.jkl");
  TEST(PathName::Compare(*component, "//abc") == 0);
  ++component;
  TEST(PathName::Compare(*component, "def") == 0);
  ++component;
  TEST(PathName::Compare(*component, "ghi.jkl") == 0);
  ++component;
  TEST(!component);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(4);
{
  PathNameParser component("/abc///def/ghi.jkl");
  TEST(PathName::Compare(*component, "/abc") == 0);
  ++component;
  TEST(PathName::Compare(*component, "def") == 0);
  ++component;
  TEST(PathName::Compare(*component, "ghi.jkl") == 0);
  ++component;
  TEST(!component);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(5);
{
  PathName path;

#if defined(MIKTEX_WINDOWS)
  path = "C:/abc/def/../ghi.jkl";
  path.MakeAbsolute();
  TEST(PathName::Compare(path.GetData(), "C:/abc/ghi.jkl") == 0);
#endif

  path = "/abc/def/../ghi.jkl";
  path.MakeAbsolute();
  TEST(PathName::Compare(path.GetData(), "/abc/ghi.jkl") == 0);

  PathName path2;

  path = "abc/./def/../ghi.jkl";
  path.MakeAbsolute();

  path2.SetToCurrentDirectory();
  path2 /= "abc/ghi.jkl";

  TEST(PathName::Compare(path.GetData(), path2) == 0);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(6);
{
#if defined(MIKTEX_WINDOWS)
  PathName path("C:/abc/def/ghi.jkl/");
  TEST(path.CutOffLastComponent() == "C:/abc/def");
  TEST(path.CutOffLastComponent() == "C:/abc");
  TEST(path.CutOffLastComponent() == "C:/");
  TEST(path.CutOffLastComponent() == "C:/");
#endif
  PathName path2("/abc/def/ghi.jkl/");
  TEST(path2.CutOffLastComponent() == "/abc/def");
  TEST(path2.CutOffLastComponent() == "/abc");
  TEST(path2.CutOffLastComponent() == "/");
  TEST(path2.CutOffLastComponent() == "/");
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(7)
{
  PathName path("/abc/def/ghi.jkl/mno.pqr.stu");
  PathName path2("/abc/def/", "ghi.jkl/mno.pqr.stu");
  TEST(PathName::Compare(path, path2) == 0);
  TEST(PathName::Compare(path.GetExtension(), ".stu") == 0);
  TEST(path.HasExtension(".stu"));
  TEST(PathName::Compare(path.GetFileNameWithoutExtension(), "mno.pqr") == 0);
  TEST(PathName::Compare(path.GetExtension(), ".stu") == 0);
  path.SetExtension(".vwx");
  TEST(path.HasExtension(".vwx"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(8);
{
  PathName path1("/abc/def");
  PathName path2(L"/abc/def");
  path1 /= u8"xxx\U0000263Axxx";
  path2 /= u8"xxx\U0000263Axxx";
  TEST(path1 == path2);
#if defined(MIKTEX_WINDOWS)
  path1 /= L"yyy\U000000C3yyy";
  path2 /= u8"yyy\U000000E3yyy";
  TEST(path1 == path2);
#endif
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(9);
{
  TEST(PathName::Match("*/ka/*", "/abra/ka/da/bra"));
  TEST(PathName::Match("*/?a/da/*", "/abra/ka/da/bra"));
  TEST(PathName::Match("/abra/*", "/abra/ka/da/bra"));
  TEST(PathName::Match("*/bra", "/abra/ka/da/bra"));
  TEST(PathName::Match("/abra/*/br*", "/abra/ka/da/bra"));
  TEST(PathName::Match("*ka*", "/abra/ka/da/bra"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(10);
{
  PathName path("/abc/def/ghi.jkl");
  TEST(path.HasExtension(".jkl"));
  TEST(path.HasExtension("jkl"));
  path.AppendExtension(".jkl");
  TEST(PathName::Compare(path, "/abc/def/ghi.jkl") == 0);
  path.SetExtension(nullptr);
  TEST(PathName::Compare(path, "/abc/def/ghi") == 0);
  path.AppendExtension(".jkl");
  TEST(PathName::Compare(path, "/abc/def/ghi.jkl") == 0);
  path.AppendExtension(".mno");
  TEST(PathName::Compare(path, "/abc/def/ghi.jkl.mno") == 0);
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
  CALL_TEST_FUNCTION(9);
  CALL_TEST_FUNCTION(10);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
