/* PostScript.cpp:

   Copyright (C) 1996-2018 Christian Schenk

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

#include "config.h"

#include <miktex/Core/Quoter>

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
void PostScript::ExecuteBatch(const char* fileName)
{
  string command;
  command.reserve(BufferSizes::MaxPath + 10);
  command = '(';
  PathName pathUnx(fileName);
  pathUnx.Convert({ ConvertPathNameOption::ToUnix, ConvertPathNameOption::MakeAbsolute });
  command += pathUnx.GetData();
  command += ") run\n";
  Execute("%s", command.c_str());
}

bool AllowShellCommand(const char* lpszCommand)
{
  return false; // TODO
}

void PostScript::CopyFile(FileStream& stream, unsigned length)
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

void PostScript::ExecuteEncapsulatedPostScript(const char* fileName)
{
  FileStream epsStream;

  unsigned start = 0;
  unsigned length = 0;


  epsStream.Attach(File::Open(fileName, FileMode::Open, FileAccess::Read, false));
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

void PostScript::ConvertToEPSThread(PathName pathFile, FILE* fileIn, FILE* fileOut)
{
  // TODO
}

FILE* PostScript::ConvertToEPS(const char* fileName)
{
  int handles[2];
  if (_pipe(handles, 4096, _O_BINARY) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("pipe");
  }
  FILE* filePipeRead = fdopen(handles[0], "rb");
  if (filePipeRead == nullptr)
  {
    _close(handles[0]);
    _close(handles[1]);
    MIKTEX_FATAL_CRT_ERROR("fdopen");
  }
  FILE* filePipeWrite = fdopen(handles[1], "wb");
  if (filePipeWrite == nullptr)
  {
    fclose(filePipeRead);
    _close(handles[1]);
    MIKTEX_FATAL_CRT_ERROR("fdopen");
  }
  FILE* pFileIn;
  try
  {
    pFileIn = File::Open(fileName, FileMode::Open, FileAccess::Read, false);
  }
  catch (const exception&)
  {
    fclose(filePipeRead);
    fclose(filePipeWrite);
    throw;
  }
  thread converterThread(&PostScript::ConvertToEPSThread, this, fileName, pFileIn, filePipeWrite);
  try
  {
    thread converterThread(&PostScript::ConvertToEPSThread, this, fileName, pFileIn, filePipeWrite);
    converterThread.detach();
  }
  catch (const exception&)
  {
    fclose(filePipeRead);
    throw;
  }
  return filePipeRead;
}

void PostScript::SendHeader(const char* headerName)
{
  shared_ptr<Session> session = Session::Get();

  PathName fileName;
  if (!session->FindFile(headerName, FileType::PSHEADER, fileName))
  {
    MIKTEX_FATAL_ERROR_2(T_("Cannot find PostScript header file."), "path", headerName);
  }
  tracePS->WriteFormattedLine("libdvi", T_("Sending %s..."), Q_(fileName));
  ExecuteBatch(fileName.GetData());
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

void PostScript::DoSpecial(PsfileSpecial* psFileSpecial)
{
  PathName pathFileName;
  if (!FindGraphicsFile(psFileSpecial->GetFileName(), pathFileName))
  {
    MIKTEX_FATAL_ERROR_2(T_("Cannot find file."), "path", psFileSpecial->GetFileName());
  }

  Execute("%d %d a\n", psFileSpecial->GetX() - dviImpl->GetResolution(), psFileSpecial->GetY() - dviImpl->GetResolution());

  Execute("@beginspecial\n");

  if (psFileSpecial->HasHsize())
  {
    Execute("%d @hsize\n", psFileSpecial->GetHsize());
  }

  if (psFileSpecial->HasVsize())
  {
    Execute("%d @vsize\n", psFileSpecial->GetVsize());
  }

  if (psFileSpecial->HasHoffset())
  {
    Execute("%d @hoffset\n", psFileSpecial->GetHoffset());
  }

  if (psFileSpecial->HasVoffset())
  {
    Execute("%d @voffset\n", psFileSpecial->GetVoffset());
  }

  if (psFileSpecial->HasHscale())
  {
    Execute("%d @hscale\n", psFileSpecial->GetHscale());
  }

  if (psFileSpecial->HasVscale())
  {
    Execute("%d @vscale\n", psFileSpecial->GetVscale());
  }

  if (psFileSpecial->HasAngle())
  {
    Execute("%d @angle\n", psFileSpecial->GetAngke());
  }

  if (psFileSpecial->HasLlx())
  {
    Execute("%d @llx\n", psFileSpecial->GetLlx());
  }

  if (psFileSpecial->HasLly())
  {
    Execute("%d @lly\n", psFileSpecial->GetLly());
  }

  if (psFileSpecial->HasUrx())
  {
    Execute("%d @urx\n", psFileSpecial->GetUrx());
  }

  if (psFileSpecial->HasUry())
  {
    Execute("%d @ury\n", psFileSpecial->GetUry());
  }

  if (psFileSpecial->HasRwi())
  {
    Execute("%d @rwi\n", psFileSpecial->GetRwi());
  }

  if (psFileSpecial->HasRhi())
  {
    Execute("%d @rhi\n", psFileSpecial->GetRhi());
  }

  if (psFileSpecial->HasClipFlag())
  {
    Execute("@clip\n");
  }

  Execute("@setspecial\n");
  ExecuteEncapsulatedPostScript(pathFileName.GetData());
  Execute("@endspecial\n");
}

void PostScript::AddDefinition(PsdefSpecial* psDefSepcial)
{
  if (psDefSepcial->GetDef())
  {
    if (find(definitions.begin(), definitions.end(), psDefSepcial->GetDef()) == definitions.end())
    {
      definitions.push_back(psDefSepcial->GetDef());
    }
  }
  else if (psDefSepcial->GetFileName())
  {
    AddHeader(psDefSepcial->GetFileName());
  }
}

void PostScript::AddHeader(const char* fileName)
{
  if (find(headers.begin(), headers.end(), fileName) == headers.end())
  {
    headers.push_back(fileName);
  }
}

void PostScript::DoSpecial(DvipsSpecial* dvipsSpecial)
{
  Execute("%d %d a\n", dvipsSpecial->GetX() - dviImpl->GetResolution(), dvipsSpecial->GetY() - dviImpl->GetResolution());
  if (dvipsSpecial->GetProtection())
  {
    Execute("@beginspecial\n");
    Execute("@setspecial\n");
  }
  if (dvipsSpecial->GetString())
  {
    Execute("%s\n", dvipsSpecial->GetString());
  }
  else if (dvipsSpecial->GetFileName())
  {
    PathName filename;
    if (!FindGraphicsFile(dvipsSpecial->GetFileName(), filename))
    {
      MIKTEX_FATAL_ERROR_2(T_("Cannot find graphics file."), "path", dvipsSpecial->GetFileName());
    }
    ExecuteEncapsulatedPostScript(filename.GetData());
  }
  if (dvipsSpecial->GetProtection())
  {
    Execute("@endspecial\n");
  }
}

inline int pt2sp(int pt)
{
  return static_cast<int>((pt * (65536 * 72.27)) / 72);
}

void PostScript::Open(DviImpl* dviImpl, int shrinkFactor)
{
  this->dviImpl = dviImpl;
  this->shrinkFactor = shrinkFactor;

  int width, height;

  // calculate device dimensions
  PaperSizeInfo size = dviImpl->GetPaperSizeInfo();
  if (dviImpl->Landscape())
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
  Execute("%d %d %d %d %d (test.dvi) @start\n", width, height, dviImpl->GetMagnification(), dviImpl->GetResolution(), dviImpl->GetResolution());

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

void PostScript::Uncompress(const char* fileName, PathName& result)
{
  PathName source;
  if (!dviImpl->FindGraphicsFile(fileName, source))
  {
    MIKTEX_FATAL_ERROR_2(T_("Cannot find graphics file."), "path", fileName);
  }
  Utils::UncompressFile(source, result);
  dviImpl->RememberTempFile(fileName, result);
}

bool IsZFileName(const PathName& fileName)
{
  return fileName.HasExtension(".gz") || fileName.HasExtension(".bz2");
}

// Find a graphics file.  Here we will handle backtics and compressed
// files.
bool PostScript::FindGraphicsFile(const char* fileName, PathName& result)
{
  if (*fileName == '`')
  {
    if (dviImpl->TryGetTempFile(fileName, result))
    {
      return true;
    }
    else if (strncmp(fileName + 1, "gunzip -c ", 10) == 0)
    {
      Uncompress(fileName + 11, result);
      return true;
    }
    else if (strncmp(fileName + 1, "bunzip2 -c ", 11) == 0)
    {
      Uncompress(fileName + 12, result);
      return true;
    }
    else if (!AllowShellCommand(fileName + 1))
    {
      tracePS->WriteFormattedLine("libdvi", T_("Ignoring shell command %s"), Q_(fileName + 1));
      return false;
    }
    else
    {
      char szTempFileName[_MAX_PATH];
      StringUtil::CopyString(szTempFileName, _MAX_PATH, PathName().SetToTempFile().GetData());
      string command;
      command = fileName + 1;
      command += " > ";
      command += Q_(szTempFileName);
      MIKTEX_ASSERT(dviImpl != nullptr);
      PathName docDir = dviImpl->GetDviFileName();
      docDir.RemoveFileSpec();
      StdoutReader reader;
      bool done = Process::ExecuteSystemCommand(command.c_str(), nullptr, &reader, docDir.GetData());
      if (!done)
      {
        // FIXME:hard-coded string
        File::Delete(szTempFileName);
        MIKTEX_FATAL_ERROR_2(T_("Execution of an embedded shell command failed for some reason."), "command", command);
      }
      dviImpl->RememberTempFile(fileName + 1, szTempFileName);
      result = szTempFileName;
      return true;
    }
  }
  else if (IsZFileName(fileName))
  {
    if (dviImpl->TryGetTempFile(fileName, result))
    {
      return true;
    }
    Uncompress(fileName, result);
    return true;
  }
  else
  {
    return dviImpl->FindGraphicsFile(fileName, result);
  }
}
