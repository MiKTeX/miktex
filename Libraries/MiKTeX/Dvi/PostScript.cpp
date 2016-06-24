/* PostScript.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#include "StdAfx.h"

#include "internal.h"

#if defined(_MSC_VER)
#include <io.h> // FIXME: see _pipe()
#include <fcntl.h> // FIXME: see _pipe()
#endif

PostScript::PostScript() :
  tracePS(TraceStream::Open(MIKTEX_TRACE_DVIPOSTSCRIPT))
{
}

PostScript::~PostScript()
{
}

void PostScript::Initialize()
{
  AddHeader("tex.pro");
#if 0
  AddHeader("finclude.pro");
#endif
  AddHeader("special.pro");
  AddHeader("gs_permitfilereading.pro");
}

// Process a PostScript file. This is done with the help of 'run',
// which seems to be a Ghostscript extension.
void PostScript::ExecuteBatch(const char * lpszFileName)
{
  string command;
  command.reserve(BufferSizes::MaxPath + 10);
  command = '(';
  PathName pathUnx(lpszFileName);
  pathUnx.Convert({ ConvertPathNameOption::ToUnix, ConvertPathNameOption::MakeAbsolute });
  command += pathUnx.Get();
  command += ") run\n";
  Execute("%s", command.c_str());
}

bool AllowShellCommand(const char * lpszCommand)
{
  return false; // TODO
}

void PostScript::CopyFile(FileStream & stream, unsigned length)
{
  MIKTEX_ASSERT(IsOpen());
  if (length == 0)
  {
    length = UINT_MAX;
  }
  size_t n;
  char buf[4096];
  while ((n = stream.Read(buf, min(4096, length))) > 0)
  {
    Write(buf, static_cast<unsigned>(n));
    length -= static_cast<unsigned>(n);
  }
}

void PostScript::ExecuteEncapsulatedPostScript(const char * lpszFileName)
{
  FileStream epsStream;

  unsigned start = 0;
  unsigned length = 0;


  epsStream.Attach(File::Open(lpszFileName, FileMode::Open, FileAccess::Read, false));
  struct
  {
    unsigned char magic[4];
    unsigned char start[4];
    unsigned char length[4];
  } epsfheader;
  if (epsStream.Read(&epsfheader, sizeof(epsfheader)) == sizeof(epsfheader)
    && epsfheader.magic[0] == 'E' + 0x80
    && epsfheader.magic[1] == 'P' + 0x80
    && epsfheader.magic[2] == 'S' + 0x80
    && epsfheader.magic[3] == 'F' + 0x80)
  {
    start = epsfheader.start[0];
    start += epsfheader.start[1] * 256;
    start += epsfheader.start[2] * 256 * 256;
    start += epsfheader.start[3] * 256 * 256 * 256;
    length = epsfheader.length[0];
    length += epsfheader.length[1] * 256;
    length += epsfheader.length[2] * 256 * 256;
    length += epsfheader.length[3] * 256 * 256 * 256;
    tracePS->WriteFormattedLine("libdvi", T_("EPS has a binary header"));
    tracePS->WriteFormattedLine("libdvi", T_("start: %u"), static_cast<unsigned>(start));
    tracePS->WriteFormattedLine("libdvi", T_("length: %u"), static_cast<unsigned>(length));
  }

  epsStream.Seek(start, SeekOrigin::Begin);

  CopyFile(epsStream, length);
}

void PostScript::ConvertToEPSThread(PathName pathFile, FILE * pfileIn, FILE * pFileOut)
{
  // TODO
}

FILE * PostScript::ConvertToEPS(const char * lpszFileName)
{
  int handles[2];
  if (_pipe(handles, 4096, _O_BINARY) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("pipe");
  }
  FILE * pFilePipeRead = fdopen(handles[0], "rb");
  if (pFilePipeRead == nullptr)
  {
    _close(handles[0]);
    _close(handles[1]);
    MIKTEX_FATAL_CRT_ERROR("fdopen");
  }
  FILE * pFilePipeWrite = fdopen(handles[1], "wb");
  if (pFilePipeWrite == nullptr)
  {
    fclose(pFilePipeRead);
    _close(handles[1]);
    MIKTEX_FATAL_CRT_ERROR("fdopen");
  }
  FILE * pFileIn;
  try
  {
    pFileIn = File::Open(lpszFileName, FileMode::Open, FileAccess::Read, false);
  }
  catch (const exception &)
  {
    fclose(pFilePipeRead);
    fclose(pFilePipeWrite);
    throw;
  }
  thread converterThread(&PostScript::ConvertToEPSThread, this, lpszFileName, pFileIn, pFilePipeWrite);
  try
  {
    thread converterThread(&PostScript::ConvertToEPSThread, this, lpszFileName, pFileIn, pFilePipeWrite);
    converterThread.detach();
  }
  catch (const exception &)
  {
    fclose(pFilePipeRead);
    throw;
  }
  return pFilePipeRead;
}

void PostScript::SendHeader(const char * lpszHeaderName)
{
  shared_ptr<Session> session = Session::Get();

  PathName fileName;
  if (!session->FindFile(lpszHeaderName, FileType::PSHEADER, fileName))
  {
    MIKTEX_FATAL_ERROR_2(T_("Cannot find PostScript header file."), "path", lpszHeaderName);
  }
  tracePS->WriteFormattedLine("libdvi", T_("Sending %s..."), Q_(fileName));
  ExecuteBatch(fileName.Get());
}

void PostScript::DoProlog()
{
  vector<string>::iterator it;
  for (it = headers.begin(); it != headers.end(); ++it)
  {
    SendHeader(it->c_str());
  }
  DoDefinitions();
}

void PostScript::DoDefinitions()
{
  if (definitions.empty())
  {
    return;
  }
  Execute("\nTeXDict begin @defspecial\n");
  for (vector<string>::iterator it = definitions.begin(); it != definitions.end(); ++it)
  {
    Execute("%s\n", it->c_str());
  }
  Execute("\n@fedspecial end\n");
}

void PostScript::DoSpecial(PsfileSpecial * ppsfilespecial)
{
  PathName pathFileName;
  if (!FindGraphicsFile(ppsfilespecial->GetFileName(), pathFileName))
  {
    MIKTEX_FATAL_ERROR_2(T_("Cannot find file."), "path", ppsfilespecial->GetFileName());
  }

  Execute("%d %d a\n", ppsfilespecial->GetX() - pDviImpl->GetResolution(), ppsfilespecial->GetY() - pDviImpl->GetResolution());

  Execute("@beginspecial\n");

  if (ppsfilespecial->HasHsize())
  {
    Execute("%d @hsize\n", ppsfilespecial->GetHsize());
  }

  if (ppsfilespecial->HasVsize())
  {
    Execute("%d @vsize\n", ppsfilespecial->GetVsize());
  }

  if (ppsfilespecial->HasHoffset())
  {
    Execute("%d @hoffset\n", ppsfilespecial->GetHoffset());
  }

  if (ppsfilespecial->HasVoffset())
  {
    Execute("%d @voffset\n", ppsfilespecial->GetVoffset());
  }

  if (ppsfilespecial->HasHscale())
  {
    Execute("%d @hscale\n", ppsfilespecial->GetHscale());
  }

  if (ppsfilespecial->HasVscale())
  {
    Execute("%d @vscale\n", ppsfilespecial->GetVscale());
  }

  if (ppsfilespecial->HasAngle())
  {
    Execute("%d @angle\n", ppsfilespecial->GetAngke());
  }

  if (ppsfilespecial->HasLlx())
  {
    Execute("%d @llx\n", ppsfilespecial->GetLlx());
  }

  if (ppsfilespecial->HasLly())
  {
    Execute("%d @lly\n", ppsfilespecial->GetLly());
  }

  if (ppsfilespecial->HasUrx())
  {
    Execute("%d @urx\n", ppsfilespecial->GetUrx());
  }

  if (ppsfilespecial->HasUry())
  {
    Execute("%d @ury\n", ppsfilespecial->GetUry());
  }

  if (ppsfilespecial->HasRwi())
  {
    Execute("%d @rwi\n", ppsfilespecial->GetRwi());
  }

  if (ppsfilespecial->HasRhi())
  {
    Execute("%d @rhi\n", ppsfilespecial->GetRhi());
  }

  if (ppsfilespecial->HasClipFlag())
  {
    Execute("@clip\n");
  }

  Execute("@setspecial\n");
  ExecuteEncapsulatedPostScript(pathFileName.Get());
  Execute("@endspecial\n");
}

void PostScript::AddDefinition(PsdefSpecial * ppsdefspecial)
{
  if (ppsdefspecial->GetDef())
  {
    if (find(definitions.begin(), definitions.end(), ppsdefspecial->GetDef()) == definitions.end())
    {
      definitions.push_back(ppsdefspecial->GetDef());
    }
  }
  else if (ppsdefspecial->GetFileName())
  {
    AddHeader(ppsdefspecial->GetFileName());
  }
}

void PostScript::AddHeader(const char * lpszFileName)
{
  if (find(headers.begin(), headers.end(), lpszFileName) == headers.end())
  {
    headers.push_back(lpszFileName);
  }
}

void PostScript::DoSpecial(DvipsSpecial * pdvipsspecial)
{
  Execute("%d %d a\n", pdvipsspecial->GetX() - pDviImpl->GetResolution(), pdvipsspecial->GetY() - pDviImpl->GetResolution());
  if (pdvipsspecial->GetProtection())
  {
    Execute("@beginspecial\n");
    Execute("@setspecial\n");
  }
  if (pdvipsspecial->GetString())
  {
    Execute("%s\n", pdvipsspecial->GetString());
  }
  else if (pdvipsspecial->GetFileName())
  {
    PathName filename;
    if (!FindGraphicsFile(pdvipsspecial->GetFileName(), filename))
    {
      MIKTEX_FATAL_ERROR_2(T_("Cannot find graphics file."), "path", pdvipsspecial->GetFileName());
    }
    ExecuteEncapsulatedPostScript(filename.Get());
  }
  if (pdvipsspecial->GetProtection())
  {
    Execute("@endspecial\n");
  }
}

inline int pt2sp(int pt)
{
  return static_cast<int>((pt * (65536 * 72.27)) / 72);
}

void PostScript::Open(DviImpl * pDviImpl, int shrinkFactor)
{
  this->pDviImpl = pDviImpl;
  this->shrinkFactor = shrinkFactor;

  int width, height;

  // calculate device dimensions
  PaperSizeInfo size = pDviImpl->GetPaperSizeInfo();
  if (pDviImpl->Landscape())
  {
    height = pt2sp(size.width);
    width = pt2sp(size.height);
  }
  else
  {
    width = pt2sp(size.width);
    height = pt2sp(size.height);
  }

  // initialize
  Initialize();
  DoProlog();
  Execute("TeXDict begin\n");
  Execute("%d %d %d %d %d (test.dvi) @start\n", width, height, pDviImpl->GetMagnification(), pDviImpl->GetResolution(), pDviImpl->GetResolution());

  openFlag = true;
}

void PostScript::Close()

{
  MIKTEX_ASSERT(!pageBegunFlag);
  openFlag = false;
  Execute("end\n");
  Finalize();
}

void PostScript::BeginPage()
{
  MIKTEX_ASSERT(!pageBegunFlag);
  Execute("1 0 bop\n");
  pageBegunFlag = true;
}

void PostScript::EndPage()
{
  MIKTEX_ASSERT(pageBegunFlag);
  Execute("eop\n");
  pageBegunFlag = false;
}

void PostScript::Finalize()
{
}

void PostScript::Uncompress(const char * lpszFileName, PathName & result)
{
  PathName source;
  if (!pDviImpl->FindGraphicsFile(lpszFileName, source))
  {
    MIKTEX_FATAL_ERROR_2(T_("Cannot find graphics file."), "path", lpszFileName);
  }
  Utils::UncompressFile(source.Get(), result);
  pDviImpl->RememberTempFile(lpszFileName, result);
}

bool IsZFileName(const PathName & fileName)
{
  return fileName.HasExtension(".gz") || fileName.HasExtension(".bz2");
}

// Find a graphics file.  Here we will handle backtics and compressed
// files.
bool PostScript::FindGraphicsFile(const char * lpszFileName, PathName & result)
{
  if (*lpszFileName == '`')
  {
    if (pDviImpl->TryGetTempFile(lpszFileName, result))
    {
      return true;
    }
    else if (strncmp(lpszFileName + 1, "gunzip -c ", 10) == 0)
    {
      Uncompress(lpszFileName + 11, result);
      return true;
    }
    else if (strncmp(lpszFileName + 1, "bunzip2 -c ", 11) == 0)
    {
      Uncompress(lpszFileName + 12, result);
      return true;
    }
    else if (!AllowShellCommand(lpszFileName + 1))
    {
      tracePS->WriteFormattedLine("libdvi", T_("Ignoring shell command %s"), Q_(lpszFileName + 1));
      return false;
    }
    else
    {
      char szTempFileName[_MAX_PATH];
      StringUtil::CopyString(szTempFileName, _MAX_PATH, PathName().SetToTempFile().Get());
      string command;
      command = lpszFileName + 1;
      command += " > ";
      command += Q_(szTempFileName);
      MIKTEX_ASSERT(pDviImpl != nullptr);
      PathName docDir = pDviImpl->GetDviFileName();
      docDir.RemoveFileSpec();
      StdoutReader reader;
      bool done = Process::ExecuteSystemCommand(command.c_str(), nullptr, &reader, docDir.Get());
      if (!done)
      {
        // FIXME:hard-coded string
        File::Delete(szTempFileName);
        MIKTEX_FATAL_ERROR_2(T_("Execution of an embedded shell command failed for some reason."), "command", command);
      }
      pDviImpl->RememberTempFile(lpszFileName + 1, szTempFileName);
      result = szTempFileName;
      return true;
    }
  }
  else if (IsZFileName(lpszFileName))
  {
    if (pDviImpl->TryGetTempFile(lpszFileName, result))
    {
      return true;
    }
    Uncompress(lpszFileName, result);
    return true;
  }
  else
  {
    return pDviImpl->FindGraphicsFile(lpszFileName, result);
  }
}
