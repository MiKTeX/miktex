/**
 * @file test/filesystem/1.cpp
 * @author Christian Schenk
 * @brief Unit tests for file system
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

#if defined(_WIN32)
#  include <Windows.h>
#endif

#include <chrono>
#include <thread>

#include <cstdio>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileSystemWatcher>
#include <miktex/Core/Utils>
#include <miktex/Util/PathName>

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;

BEGIN_TEST_SCRIPT("filesystem-1");

BEGIN_TEST_FUNCTION(1);
{
    TEST(!MiKTeX::Core::File::Exists(PathName("xxx.zzz")));
    Touch("xxx.zzz");
#if defined(MIKTEX_UNIX)
    LOG4CXX_INFO(logger, "xxx.zzz permissions: " << std::oct << MiKTeX::Core::File::GetNativeAttributes(PathName("xxx.zzz")));
#endif
    TEST(MiKTeX::Core::File::Exists(PathName("xxx.zzz")));
    TESTX(MiKTeX::Core::File::SetAttributes(PathName("xxx.zzz"), { MiKTeX::Core::FileAttribute::ReadOnly }));
#if defined(MIKTEX_UNIX)
    LOG4CXX_INFO(logger, "xxx.zzz permissions: " << std::oct << MiKTeX::Core::File::GetNativeAttributes(PathName("xxx.zzz")));
#endif
    MiKTeX::Core::FileAttributeSet attributes = MiKTeX::Core::File::GetAttributes(PathName("xxx.zzz"));
    TEST(attributes[MiKTeX::Core::FileAttribute::ReadOnly]);
    attributes -= MiKTeX::Core::FileAttribute::ReadOnly;
    TESTX(MiKTeX::Core::File::SetAttributes(PathName("xxx.zzz"), attributes));
#if defined(MIKTEX_UNIX)
    LOG4CXX_INFO(logger, "xxx.zzz permissions: " << std::oct << MiKTeX::Core::File::GetNativeAttributes(PathName("xxx.zzz")));
#endif
    attributes = MiKTeX::Core::File::GetAttributes(PathName("xxx.zzz"));
    TEST(!attributes[MiKTeX::Core::FileAttribute::ReadOnly]);
    TESTX(MiKTeX::Core::File::Move(PathName("xxx.zzz"), PathName("zzz.xxx")));
    TESTX(MiKTeX::Core::File::Delete(PathName("zzz.xxx")));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
    FILE* stream = MiKTeX::Core::File::Open(PathName("abc.def"), MiKTeX::Core::FileMode::Create, MiKTeX::Core::FileAccess::Write, true);
    TEST(stream != nullptr);
    fprintf(stream, "hello, world!\n");
    fclose(stream);
    TESTX(MiKTeX::Core::File::Delete(PathName("abc.def")));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
    MiKTeX::Util::PathName dir;
    dir.SetToCurrentDirectory();
    dir /= "d";
    MiKTeX::Util::PathName subdir(dir);
    subdir /= "dd/ddd";
    TESTX(MiKTeX::Core::Directory::Create(subdir));
    TEST(MiKTeX::Core::Directory::Exists(subdir));
    TEST(!MiKTeX::Core::File::Exists(subdir));
    MiKTeX::Core::FileAttributeSet attributes = MiKTeX::Core::File::GetAttributes(subdir);
    TEST(attributes[MiKTeX::Core::FileAttribute::Directory]);
    MiKTeX::Util::PathName file = subdir;
    file /= "file.txt";
    Touch(file.GetData());
    TEST(MiKTeX::Core::File::Exists(file));
    MiKTeX::Util::PathName dir2;
    dir2.SetToCurrentDirectory();
    dir2 /= "copy-of-d";
    TESTX(MiKTeX::Core::Directory::Copy(dir, dir2, { MiKTeX::Core::DirectoryCopyOption::CopySubDirectories }));
    MiKTeX::Util::PathName file2 = dir2;
    file2 /= "dd/ddd";
    file2 /= "file.txt";
    TEST(MiKTeX::Core::File::Equals(file, file2));
    TESTX(MiKTeX::Core::Directory::Delete(dir, true));
    TESTX(MiKTeX::Core::Directory::Delete(dir2, true));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(4);
{
    Touch("abrakadabra.txt");
    MiKTeX::Core::File::Move(PathName("abrakadabra.txt"), PathName("abrakadabra.txt.tmp"));
    TEST(!MiKTeX::Core::File::Exists(PathName("abrakadabra.txt")));
    MiKTeX::Core::File::Delete(PathName("abrakadabra.txt.tmp"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(5);
{
    TEST(Utils::IsSafeFileName(PathName("sample.tex")));
    TEST(Utils::IsSafeFileName(PathName("./sample.tex")));
    TEST(Utils::IsSafeFileName(PathName(".tex")));
    TEST(Utils::IsSafeFileName(PathName("sub/dir/sample.tex")));
    TEST(Utils::IsSafeFileName(PathName("sub/dir/.tex")));
    TEST(!Utils::IsSafeFileName(PathName(".bashrc")));
    TEST(!Utils::IsSafeFileName(PathName("sub/dir/.bashrc")));
    TEST(!Utils::IsSafeFileName(PathName("/fully/qualified/dir/sample.tex")));
    TEST(!Utils::IsSafeFileName(PathName("/fully/qualified/dir/.bashrc")));
    TEST(!Utils::IsSafeFileName(PathName("../sample.tex")));
    TEST(!Utils::IsSafeFileName(PathName("../.bashrc")));
    TEST(!Utils::IsSafeFileName(PathName("sub/dir/../../../sample.tex")));
    TEST(!Utils::IsSafeFileName(PathName("sub/dir/../../../.bashrc")));
    TEST(Utils::IsSafeFileName(PathName("sub/dir/../../sample.tex")));
}
END_TEST_FUNCTION();

#if defined(MIKTEX_WINDOWS)
BEGIN_TEST_FUNCTION(6);
{
    wchar_t szPath[BufferSizes::MaxPath];
    DWORD n = GetModuleFileNameW(nullptr, szPath, BufferSizes::MaxPath);
    TEST(n > 0 && n < MiKTeX::Core::BufferSizes::MaxPath);
    MiKTeX::Util::PathName myself(szPath);
    bool deletedMyself;
    try
    {
        MiKTeX::Core::File::Delete(myself);
        deletedMyself = true;
    }
    catch (const MiKTeX::Core::MiKTeXException& ex)
    {
        TEST(dynamic_cast<const MiKTeX::Core::UnauthorizedAccessException*>(&ex) != nullptr);
        deletedMyself = false;
    }
    TEST(!deletedMyself);
    MiKTeX::Util::PathName clone(myself);
    clone.AppendExtension(".tmp");
    TESTX(MiKTeX::Core::File::Move(myself, clone));
    TEST(!MiKTeX::Core::File::Exists(myself));
    Touch(myself.GetData());
    TEST(MiKTeX::Core::File::Exists(myself));
    MiKTeX::Core::File::Delete(myself);
    TESTX(MiKTeX::Core::File::Move(clone, myself));
    TEST(MiKTeX::Core::File::Exists(myself));
    TEST(MiKTeX::Core::File::Exists(PathName("nul")));
}
END_TEST_FUNCTION();
#endif

BEGIN_TEST_FUNCTION(7);
{
    MiKTeX::Util::PathName cd;
    cd.SetToCurrentDirectory();
    MiKTeX::Util::PathName dir = cd / "long-path-parent-directory";
    MiKTeX::Util::PathName longPath(dir);
    const int minLength = 900;
    for (int n = 0; longPath.GetLength() < minLength; ++n)
    {
        longPath /= ("abcdefghij-"s + std::to_string(n));
    }
    TESTX(MiKTeX::Core::Directory::Create(longPath));
    TEST(MiKTeX::Core::Directory::Exists(longPath));
    MiKTeX::Util::PathName file = longPath / "file.txt";
    Touch(file.GetData());
    TEST(MiKTeX::Core::File::Exists(file));
    TESTX(MiKTeX::Core::File::Delete(file));
    TEST(!MiKTeX::Core::File::Exists(file));
    TESTX(MiKTeX::Core::Directory::Delete(dir, true));
    TEST(!MiKTeX::Core::Directory::Exists(dir));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(8);
{
    class ChangeHandler : public FileSystemWatcherCallback
    {
    public:
        void OnChange(const FileSystemChangeEvent &ev) override {
        if (ev.fileName.GetFileName() == PathName("8.txt"))
        {
            switch (ev.action)
            {
            case FileSystemChangeAction::Added: added = true; break;
            case FileSystemChangeAction::Modified: modified = true; break;
            case FileSystemChangeAction::Removed: removed = true; break;
            }
        }
        };
        bool added = false;
        bool modified = false;
        bool removed = false;
    };
    ChangeHandler handler;
    auto watcher = FileSystemWatcher::Create();
    watcher->Start();
    PathName dir;
    dir.SetToCurrentDirectory();
    watcher->AddDirectories({dir});
    dir.SetToTempDirectory();
    watcher->AddDirectories({dir});
    this_thread::sleep_for(chrono::seconds(1));
    watcher->Subscribe(&handler);
    Touch("8.txt");
    this_thread::sleep_for(chrono::seconds(1));
    Touch("8.txt");
    this_thread::sleep_for(chrono::seconds(1));
    File::Delete(PathName("8.txt"));
    this_thread::sleep_for(chrono::seconds(1));
    TESTX(watcher->Stop());
    TESTX(watcher = nullptr);
    TEST(handler.added);
    TEST(handler.modified);
    TEST(handler.removed);
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
    CALL_TEST_FUNCTION(1);
    CALL_TEST_FUNCTION(2);
    CALL_TEST_FUNCTION(3);
    CALL_TEST_FUNCTION(4);
    CALL_TEST_FUNCTION(5);
#if defined(MIKTEX_WINDOWS)
    CALL_TEST_FUNCTION(6);
#endif
    CALL_TEST_FUNCTION(7);
    CALL_TEST_FUNCTION(8);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
