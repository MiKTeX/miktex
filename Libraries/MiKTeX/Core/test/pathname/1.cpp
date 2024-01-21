/**
 * @file test/pathname/1.cpp
 * @author Christian Schenk
 * @brief Unit tests for path names
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <miktex/Core/Test>
#include <miktex/Core/Utils>

#include <miktex/Util/PathName>
#include <miktex/Util/PathNameParser>

#if defined(MIKTEX_WINDOWS)
#include <direct.h>
#endif

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;

BEGIN_TEST_SCRIPT("pathname-1");

BEGIN_TEST_FUNCTION(1);
{
    TEST(PathName::Compare("//aBc/[e]/ghi.jkl", "//aBc/[e]/qwe.rty", 10) == 0);
    TEST(PathName::Compare("/abc/def/ghi.jkl", "/abc/def/qwe.rty", 10) != 0);
    PathName path("/abc/def/ghi.jkl");
    TEST(PathName::Compare(path.GetDirectoryName(), PathName("/abc/def")) == 0);
    TEST(PathName::Compare(path.GetDirectoryName(), PathName("/abc/def/")) == 0);
    TEST(PathName::Compare(path.GetFileName(), PathName("ghi.jkl")) == 0);
    TEST(PathName::Compare(path.GetFileNameWithoutExtension(), PathName("ghi")) == 0);
    TEST(PathName::Compare(path.GetExtension(), ".jkl") == 0);
    TEST(PathName::Compare(path.GetExtension(), "jkl") != 0);
    PathName path2 = path;
    path2.RemoveDirectorySpec();
    TEST(path2 == PathName("ghi.jkl"));
    TEST(PathName::Compare(PathName("/a/b/c"), PathName("/a/////b/c")) == 0);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
#if defined(MIKTEX_WINDOWS)
    PathNameParser component(PathName("C:/abc/def/ghi.jkl"));
    TEST(PathName::Compare(*component, "C:") == 0);
    ++component;
    TEST(PathName::Compare(*component, "/") == 0);
    ++component;
    TEST(PathName::Compare(*component, "abc") == 0);
    ++component;
    TEST(PathName::Compare(*component, "def") == 0);
    ++component;
    TEST(PathName::Compare(*component, "ghi.jkl") == 0);
    ++component;
    TEST(!component);
#endif
    PathNameParser component2(PathName("/abc/def/ghi.jkl"));
    TEST(PathName::Compare(*component2, "/") == 0);
    ++component2;
    TEST(PathName::Compare(*component2, "abc") == 0);
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
    PathNameParser component(PathName("//abc/def/ghi.jkl"));
    TEST(PathName::Compare(*component, "//abc") == 0);
    ++component;
    TEST(PathName::Compare(*component, "/") == 0);
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
    PathNameParser component(PathName("/abc///def/ghi.jkl"));
    TEST(PathName::Compare(*component, "/") == 0);
    ++component;
    TEST(PathName::Compare(*component, "abc") == 0);
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
    path.MakeFullyQualified();
    TEST(PathName::Compare(path.GetData(), "C:/abc/ghi.jkl") == 0);
#endif

    path = "/abc/def/../ghi.jkl";
    path.MakeFullyQualified();
#if defined(MIKTEX_WINDOWS)
    char currentDriveLetter = _getdrive() + 'A' - 1;
    TEST(PathName::Compare(path.ToString(), string(1, currentDriveLetter) + ":/abc/ghi.jkl"s) == 0);
#else
    TEST(PathName::Compare(path.GetData(), "/abc/ghi.jkl") == 0);
#endif

    PathName path2;

    path = "abc/./def/../ghi.jkl";
    path.MakeFullyQualified();

    path2.SetToCurrentDirectory();
    path2 /= "abc/ghi.jkl";

    TEST(PathName::Compare(path, path2) == 0);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(6);
{
#if defined(MIKTEX_WINDOWS)
    PathName path("C:/abc/def/ghi.jkl/");
    TEST(path.CutOffLastComponent() == PathName("C:/abc/def"));
    TEST(path.CutOffLastComponent() == PathName("C:/abc"));
    TEST(path.CutOffLastComponent() == PathName("C:/"));
    TEST(path.CutOffLastComponent() == PathName("C:/"));
#endif
    PathName path2("/abc/def/ghi.jkl/");
    TEST(path2.CutOffLastComponent() == PathName("/abc/def"));
    TEST(path2.CutOffLastComponent() == PathName("/abc"));
    TEST(path2.CutOffLastComponent() == PathName("/"));
    TEST(path2.CutOffLastComponent() == PathName("/"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(7);
{
    PathName path("/abc/def/ghi.jkl/mno.pqr.stu");
    PathName path2("/abc/def/", "ghi.jkl/mno.pqr.stu");
    TEST(PathName::Compare(path, path2) == 0);
    TEST(PathName::Compare(path.GetExtension(), ".stu") == 0);
    TEST(path.HasExtension(".stu"));
    TEST(PathName::Compare(path.GetFileNameWithoutExtension(), PathName("mno.pqr")) == 0);
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
    path1 /= PathName(L"yyy\U000000C3yyy");
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
    TEST(PathName::Compare(path, PathName("/abc/def/ghi.jkl")) == 0);
    path.SetExtension(nullptr);
    TEST(PathName::Compare(path, PathName("/abc/def/ghi")) == 0);
    path.AppendExtension(".jkl");
    TEST(PathName::Compare(path, PathName("/abc/def/ghi.jkl")) == 0);
    path.AppendExtension(".mno");
    TEST(PathName::Compare(path, PathName("/abc/def/ghi.jkl.mno")) == 0);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(11);
{
    PathName path("/abc/def/ghi.jkl");
    for (int i = 0; i < 100; ++i)
    {
        path /= "abrakadabra";
    }
    PathName path2 = std::move(path);
    TEST(path.Empty());
    PathName path3;
    path3 = std::move(path2);
    TEST(path2.Empty());
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(12);
{
    vector<string> vec;
#if defined(MIKTEX_WINDOWS)
    vec = PathName::Split(PathName("C:/abc/def/ghi.jkl"));
    TEST(vec.size() == 5);
    TEST(PathName::Compare(vec[0], "C:") == 0);
    TEST(PathName::Compare(vec[1], "/") == 0);
    TEST(PathName::Compare(vec[2], "abc") == 0);
    TEST(PathName::Compare(vec[3], "def") == 0);
    TEST(PathName::Compare(vec[4], "ghi.jkl") == 0);
#endif
    vec = PathName::Split(PathName("//server/abc/def/ghi.jkl"));
    TEST(vec.size() == 5);
    TEST(PathName::Compare(vec[0], "//server") == 0);
    TEST(PathName::Compare(vec[1], "/") == 0);
    TEST(PathName::Compare(vec[2], "abc") == 0);
    TEST(PathName::Compare(vec[3], "def") == 0);
    TEST(PathName::Compare(vec[4], "ghi.jkl") == 0);
    vec = PathName::Split(PathName("/abc/def/ghi.jkl"));
    TEST(vec.size() == 4);
    TEST(PathName::Compare(vec[0], "/") == 0);
    TEST(PathName::Compare(vec[1], "abc") == 0);
    TEST(PathName::Compare(vec[2], "def") == 0);
    TEST(PathName::Compare(vec[3], "ghi.jkl") == 0);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(13);
{
    PathName prefix;
    TEST(Utils::GetPathNamePrefix(PathName("/abc/def/ghi/jkl"), PathName("ghi/jkl"), prefix));
    TEST(prefix == PathName("/abc/def"));
    TEST(!Utils::GetPathNamePrefix(PathName("/abc/def/ghi/jkl"), PathName("ghi/jkl/foo.bar"), prefix));
}
END_TEST_FUNCTION();

#if defined(MIKTEX_WINDOWS)
BEGIN_TEST_FUNCTION(14);
{
    TEST(MiKTeX::Util::PathName("C:/Foo/bar/FooBar.txt").ToExtendedLengthPathName() == PathName("\\\\?\\C:\\Foo\\bar\\FooBar.txt"));
    PathName longPathRel("rel");
    for (int n = 0; n < 100; ++n)
    {
        longPathRel /= "abcdefghij-"s + std::to_string(n);
    }
    TEST(MiKTeX::Util::PathNameUtil::IsAbsolutePath(longPathRel.ToExtendedLengthPathName().ToString()));
    TEST(MiKTeX::Util::PathName("nul").ToExtendedLengthPathName() == PathName("\\\\.\\nul"));
}
END_TEST_FUNCTION();
#endif

BEGIN_TEST_FUNCTION(15);
{
    const char* lpszShortSourceFile;
#if defined(MIKTEX_WINDOWS)
    lpszShortSourceFile = Utils::GetRelativizedPath(
        "C:\\jenkins\\workspace\\miktex\\windows\\build\\source\\Libraries\\MiKTeX\\Core\\Utils\\Utils.cpp",
        "C:/jenkins/workspace/miktex/windows/build/source");
    TEST(lpszShortSourceFile != nullptr);
    TEST(strcmp(lpszShortSourceFile, "Libraries\\MiKTeX\\Core\\Utils\\Utils.cpp") == 0);
#endif
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
    CALL_TEST_FUNCTION(11);
    CALL_TEST_FUNCTION(12);
    CALL_TEST_FUNCTION(13);
#if defined(MIKTEX_WINDOWS)
    CALL_TEST_FUNCTION(14);
#endif
    CALL_TEST_FUNCTION(15);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
