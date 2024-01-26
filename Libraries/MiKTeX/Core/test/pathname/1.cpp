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
    TEST(PathName::ComparePrefixes(PathName("//aBc/[e]/ghi.jkl"), PathName("//aBc/[e]/qwe.rty"), 10) == 0);
    TEST(PathName::ComparePrefixes(PathName("/abc/def/ghi.jkl"), PathName("/abc/def/qwe.rty"), 10) != 0);
    PathName path("/abc/def/ghi.jkl");
    TEST(PathName::Equals(path.GetDirectoryName(), PathName("/abc/def")));
    TEST(PathName::Equals(path.GetDirectoryName(), PathName("/abc/def/")));
    TEST(PathName::Equals(path.GetFileName(), PathName("ghi.jkl")));
    TEST(PathName::Equals(path.GetFileNameWithoutExtension(), PathName("ghi")));
    TEST(PathName::Equals(PathName(path.GetExtension()), PathName(".jkl")));
    TEST(!PathName::Equals(PathName(path.GetExtension()), PathName("jkl")));
    PathName path2 = path;
    path2.RemoveDirectorySpec();
    TEST(path2 == PathName("ghi.jkl"));
    auto path3 = PathName("/a/////b/../b/c");
    path3.Convert({ConvertPathNameOption::CleanUp});
    TEST(PathName::Equals(PathName("/a/b/c"), path3));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
#if defined(MIKTEX_WINDOWS)
    auto components = PathName::Split(PathName("C:/abc/def/ghi.jkl"));
    TEST(components == vector<string>({ "C:", "/", "abc", "def", "ghi.jkl" }));
#endif
    auto components2 = PathName::Split(PathName("/abc/def/ghi.jkl"));
    TEST(components2 == vector<string>({ "/", "abc", "def", "ghi.jkl" }));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
    auto components = PathName::Split(PathName("//abc/def/ghi.jkl"));
    TEST(components == vector<string>({ "//abc", "/", "def", "ghi.jkl" }));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(4);
{
    auto components = PathName::Split(PathName("/abc///def/ghi.jkl"));
    TEST(components == vector<string>({ "/", "abc", "def", "ghi.jkl" }));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(5);
{
    PathName path;

#if defined(MIKTEX_WINDOWS)
    path = "C:/abc/def/../ghi.jkl";
    path.MakeFullyQualified();
    TEST(PathName::Equals(path, PathName("C:/abc/ghi.jkl")));
#endif

    path = "/abc/def/../ghi.jkl";
    path.MakeFullyQualified();
#if defined(MIKTEX_WINDOWS)
    char currentDriveLetter = _getdrive() + 'A' - 1;
    TEST(PathName::Equals(path, PathName(string(1, currentDriveLetter) + ":/abc/ghi.jkl"s)));
#else
    TEST(PathName::Equals(path, PathName("/abc/ghi.jkl")));
#endif

    PathName path2;

    path = "abc/./def/../ghi.jkl";
    path.MakeFullyQualified();

    path2.SetToCurrentDirectory();
    path2 /= "abc/ghi.jkl";

    TEST(PathName::Equals(path, path2));
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
    PathName path2(PathName("/abc/def/") / "ghi.jkl/mno.pqr.stu");
    TEST(PathName::Equals(path, path2));
    TEST(PathName::Equals(PathName(path.GetExtension()), PathName(".stu")));
    TEST(path.HasExtension(".stu"));
    TEST(PathName::Equals(path.GetFileNameWithoutExtension(), PathName("mno.pqr")));
    TEST(PathName::Equals(PathName(path.GetExtension()), PathName(".stu")));
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
    path1 /= PathName(L"yyy\U000000C3yyy").ToString();
    path2 /= u8"yyy\U000000E3yyy";
    TEST(path1 == path2);
#endif
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(9);
{
    TEST(PathName::Match("*/ka/*", PathName("/abra/ka/da/bra")));
    TEST(PathName::Match("*/?a/da/*", PathName("/abra/ka/da/bra")));
    TEST(PathName::Match("/abra/*", PathName("/abra/ka/da/bra")));
    TEST(PathName::Match("*/bra", PathName("/abra/ka/da/bra")));
    TEST(PathName::Match("/abra/*/br*", PathName("/abra/ka/da/bra")));
    TEST(PathName::Match("*ka*", PathName("/abra/ka/da/bra")));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(10);
{
    PathName path("/abc/def/ghi.jkl");
    TEST(path.HasExtension(".jkl"));
    TEST(path.HasExtension("jkl"));
    path.AppendExtension(".jkl");
    TEST(PathName::Equals(path, PathName("/abc/def/ghi.jkl")));
    path.SetExtension("");
    TEST(PathName::Equals(path, PathName("/abc/def/ghi")));
    path.AppendExtension(".jkl");
    TEST(PathName::Equals(path, PathName("/abc/def/ghi.jkl")));
    path.AppendExtension(".mno");
    TEST(PathName::Equals(path, PathName("/abc/def/ghi.jkl.mno")));
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
    TEST(PathName::Equals(PathName(vec[0]), PathName("C:")));
    TEST(PathName::Equals(PathName(vec[1]), PathName("/")));
    TEST(PathName::Equals(PathName(vec[2]), PathName("abc")));
    TEST(PathName::Equals(PathName(vec[3]), PathName("def")));
    TEST(PathName::Equals(PathName(vec[4]), PathName("ghi.jkl")));
#endif
    vec = PathName::Split(PathName("//server/abc/def/ghi.jkl"));
    TEST(vec.size() == 5);
    TEST(PathName::Equals(PathName(vec[0]), PathName("//server")));
    TEST(PathName::Equals(PathName(vec[1]), PathName("/")));
    TEST(PathName::Equals(PathName(vec[2]), PathName("abc")));
    TEST(PathName::Equals(PathName(vec[3]), PathName("def")));
    TEST(PathName::Equals(PathName(vec[4]), PathName("ghi.jkl")));
    vec = PathName::Split(PathName("/abc/def/ghi.jkl"));
    TEST(vec.size() == 4);
    TEST(PathName::Equals(PathName(vec[0]), PathName("/")));
    TEST(PathName::Equals(PathName(vec[1]), PathName("abc")));
    TEST(PathName::Equals(PathName(vec[2]), PathName("def")));
    TEST(PathName::Equals(PathName(vec[3]), PathName("ghi.jkl")));
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
