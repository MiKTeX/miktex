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

#include "config.h"

#include <miktex/Core/Test>

#if defined(_WIN32)
#  include <Windows.h>
#endif

#include <cstdio>

#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/PathName>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

BEGIN_TEST_SCRIPT("filesystem-1");

BEGIN_TEST_FUNCTION(1);
{
  TEST(!MiKTeX::Core::File::Exists("xxx.zzz"));
  Touch("xxx.zzz");
  TEST(MiKTeX::Core::File::Exists("xxx.zzz"));
  TESTX(MiKTeX::Core::File::SetAttributes("xxx.zzz", { MiKTeX::Core::FileAttribute::ReadOnly }));
  MiKTeX::Core::FileAttributeSet attributes = MiKTeX::Core::File::GetAttributes("xxx.zzz");
  TEST(attributes[MiKTeX::Core::FileAttribute::ReadOnly]);
  attributes -= MiKTeX::Core::FileAttribute::ReadOnly;
  TESTX(MiKTeX::Core::File::SetAttributes("xxx.zzz", attributes));
  attributes = MiKTeX::Core::File::GetAttributes("xxx.zzz");
  TEST(!attributes[MiKTeX::Core::FileAttribute::ReadOnly]);
  TESTX(MiKTeX::Core::File::Move("xxx.zzz", "zzz.xxx"));
  TESTX(MiKTeX::Core::File::Delete("zzz.xxx"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  FILE* stream = MiKTeX::Core::File::Open("abc.def", MiKTeX::Core::FileMode::Create, MiKTeX::Core::FileAccess::Write, true, MiKTeX::Core::FileShare::None);
  TEST(stream != nullptr);
  fprintf(stream, "hello, world!\n");
  fclose(stream);
  TESTX(MiKTeX::Core::File::Delete("abc.def"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
  MiKTeX::Core::PathName dir;
  dir.SetToCurrentDirectory();
  dir /= "d";
  MiKTeX::Core::PathName subdir(dir);
  subdir /= "dd/ddd";
  TESTX(MiKTeX::Core::Directory::Create(subdir));
  TEST(MiKTeX::Core::Directory::Exists(subdir));
  TEST(!MiKTeX::Core::File::Exists(subdir));
  MiKTeX::Core::FileAttributeSet attributes = MiKTeX::Core::File::GetAttributes(subdir);
  TEST(attributes[MiKTeX::Core::FileAttribute::Directory]);
  MiKTeX::Core::PathName file = subdir;
  file /= "file.txt";
  Touch(file.GetData());
  TEST(MiKTeX::Core::File::Exists(file));
  MiKTeX::Core::PathName dir2;
  dir2.SetToCurrentDirectory();
  dir2 /= "copy-of-d";
  TESTX(MiKTeX::Core::Directory::Copy(dir, dir2, { MiKTeX::Core::DirectoryCopyOption::CopySubDirectories }));
  MiKTeX::Core::PathName file2 = dir2;
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
  MiKTeX::Core::File::Move("abrakadabra.txt", "abrakadabra.txt.tmp");
  TEST(!MiKTeX::Core::File::Exists("abrakadabra.txt"));
  MiKTeX::Core::File::Delete("abrakadabra.txt.tmp");
}
END_TEST_FUNCTION();

#if defined(MIKTEX_WINDOWS)
BEGIN_TEST_FUNCTION(5);
{
  wchar_t szPath[BufferSizes::MaxPath];
  DWORD n = GetModuleFileNameW(nullptr, szPath, BufferSizes::MaxPath);
  TEST(n > 0 && n < MiKTeX::Core::BufferSizes::MaxPath);
  MiKTeX::Core::PathName myself(szPath);
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
  MiKTeX::Core::PathName clone(myself);
  clone.AppendExtension(".tmp");
  TESTX(MiKTeX::Core::File::Move(myself, clone));
  TEST(!MiKTeX::Core::File::Exists(myself));
  Touch(myself.GetData());
  TEST(MiKTeX::Core::File::Exists(myself));
  MiKTeX::Core::File::Delete(myself);
  TESTX(MiKTeX::Core::File::Move(clone, myself));
  TEST(MiKTeX::Core::File::Exists(myself));
}
END_TEST_FUNCTION();
#endif

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
  CALL_TEST_FUNCTION(2);
  CALL_TEST_FUNCTION(3);
  CALL_TEST_FUNCTION(4);
#if defined(MIKTEX_WINDOWS)
  CALL_TEST_FUNCTION(5);
#endif
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
