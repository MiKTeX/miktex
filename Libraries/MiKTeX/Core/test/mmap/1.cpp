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

#include <cstring>

#include <memory>
#include <string>

#include <miktex/Core/Test>

#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/MD5>
#include <miktex/Core/MemoryMappedFile>
#include <miktex/Core/StreamReader>
#include <miktex/Core/StreamWriter>

BEGIN_TEST_SCRIPT();

BEGIN_TEST_FUNCTION(1);
{
  StreamWriter writer ("test1");
  writer.WriteLine (T_("hello, world!"));
  writer.Close ();
  unique_ptr<MemoryMappedFile> mmap (MemoryMappedFile::Create());
  TESTX (mmap->Open("test1", true));
  char * lpsz = reinterpret_cast<char *>(mmap->GetPtr());
  TEST (memcmp(lpsz, "hello, world!", 13 * sizeof(char)) == 0);
  memcpy (lpsz, "uryyb, jbeyq!", 13 * sizeof(char));
  TESTX (mmap->Close());
  StreamReader reader ("test1");
  string line;
  TEST (reader.ReadLine(line));
  reader.Close ();
  TEST (memcmp(line.c_str(), "uryyb, jbeyq!", 13 * sizeof(char)) == 0);
  TESTX (File::Delete("test1"));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  FileStream stream;
  stream.Attach (File::Open("test2", FileMode::Create, FileAccess::Write, false));
  char buf[1024];
  memset (buf, 0xcc, 1024);
  stream.Write (buf, 1024);
  stream.Close ();
  unique_ptr<MemoryMappedFile> mmap (MemoryMappedFile::Create());
  TEST (mmap->Open("test2", true) != 0);
  TEST (mmap->Resize(2048) != 0);
  memcpy (static_cast<char*>(mmap->GetPtr()) + 1024, buf, 1024);
  TESTX (mmap->Close());
  TEST (File::GetSize("test2") == 2048);
  TEST (MD5::FromFile("test2") == MD5::Parse("e36f492c295ce23fdcfbf63fd665072b"));
  TESTX (File::Delete("test2"));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION (1);
  CALL_TEST_FUNCTION (2);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT ();
