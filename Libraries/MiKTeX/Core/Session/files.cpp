/* files.cpp: file system operations

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/CommandLineBuilder.h"
#include "miktex/Core/BZip2Stream.h"
#include "miktex/Core/GzipStream.h"
#include "miktex/Core/FileStream.h"
#include "miktex/Core/LzmaStream.h"
#include "miktex/Core/PathName.h"

#include "Fndb/FileNameDatabase.h"
#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

const size_t PIPE_SIZE = 4096;

MIKTEXSTATICFUNC(int) Close(int fd)
{
  return close(fd);
}

MIKTEXSTATICFUNC(FILE *) POpen(const char * lpszCommand, const char * lpszMode)
{
  FILE * pFile;
#if defined(_MSC_VER) || defined(__MINGW32__)
  pFile = _popen(lpszCommand, lpszMode);
#else
  pFile = popen(lpszCommand, lpszMode);
#endif
  if (pFile == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR_2("popen", "command", lpszCommand, "mode", lpszMode);
  }
  return pFile;
}

MIKTEXSTATICFUNC(int) PClose(FILE * pFile)
{
  int exitCode;
#if defined(_MSC_VER) || defined(__MINGW32__)
  exitCode = _pclose(pFile);
#else
  exitCode = pclose(pFile);
#endif
  if (exitCode < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("pclose");
  }
  return exitCode;
}

static array<unique_ptr<FileStream>, 2> CreatePipe(size_t pipeSize)
{
  int handles[2];
  int p;
#if defined(_MSC_VER) || defined(__MINGW32__)
  p = _pipe(handles, static_cast<unsigned>(pipeSize), _O_BINARY);
#else
  UNUSED_ALWAYS(pipeSize);
  p = pipe(handles);
#endif
  if (p != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("pipe");
  }
  array<unique_ptr<FileStream>, 2> files;
  try
  {
    files[0] = make_unique<FileStream>(FdOpen(handles[0], "rb"));
    files[1] = make_unique<FileStream>(FdOpen(handles[1], "wb"));
  }
  catch (const exception &)
  {
    if (files[0] == nullptr)
    {
      Close(handles[0]);
    }
    if (files[1] == nullptr)
    {
      Close(handles[1]);
    }
    throw;
  }
  return files;
}

void SessionImpl::RecordFileInfo(const PathName & path, FileAccess access)
{
  if (!(recordingFileNames || recordingPackageNames || packageHistoryFile.length() > 0))
  {
    return;
  }
  fileInfoRecords.reserve(50);
  FileInfoRecord fir;
  fir.fileName = path.ToString();
  fir.access = access;
  if (recordingPackageNames || !packageHistoryFile.empty())
  {
    PathName pathRelPath;
    if (IsTEXMFFile(path, pathRelPath))
    {
      shared_ptr<FileNameDatabase> fndb = GetFileNameDatabase(GetMpmRoot());
      if (fndb != nullptr)
      {
	vector<PathName> paths;
	vector<string> packageNames;
	if (fndb->Search(pathRelPath.Get(), MPM_ROOT_PATH, true, paths, packageNames))
	{
	  fir.packageName = packageNames[0];
	}
      }
    }
  }
  fileInfoRecords.push_back(fir);
  if (fileNameRecorderStream.IsOpen())
  {
    fileNameRecorderStream.WriteFormattedLine("%s %s", fir.access == FileAccess::Read ? "INPUT" : "OUTPUT", PathName(fir.fileName).ToUnix().Get());
    fileNameRecorderStream.Flush();
  }
}

FILE * SessionImpl::TryOpenFile(const PathName & path, FileMode mode, FileAccess access, bool text)
{
  try
  {
    return OpenFile(path, mode, access, text, FileShare::Read);
  }
#if defined(MIKTEX_WINDOWS)
  catch (const SharingViolationException &)
  {
    return nullptr;
  }
#endif
  catch (const UnauthorizedAccessException &)
  {
    return nullptr;
  }
  catch (const FileNotFoundException &)
  {
    return nullptr;
  }
}

FILE * SessionImpl::OpenFile(const PathName & path, FileMode mode, FileAccess access, bool text)
{
  return OpenFile(path, mode, access, text, FileShare::Read);
}

FILE * SessionImpl::TryOpenFile(const PathName & path, FileMode mode, FileAccess access, bool text, FileShare share)
{
  try
  {
    return OpenFile(path, mode, access, text, share);
  }
#if defined(MIKTEX_WINDOWS)
  catch (const SharingViolationException &)
  {
    return nullptr;
  }
#endif
  catch (const UnauthorizedAccessException &)
  {
    return nullptr;
  }
  catch (const FileNotFoundException &)
  {
    return nullptr;
  }
}

FILE * SessionImpl::OpenFile(const PathName & path, FileMode mode, FileAccess access, bool text, FileShare share)
{
  trace_files->WriteFormattedLine("core", "OpenFile(\"%s\", %d, 0x%x, %d, %d)", path.ToString().c_str(), static_cast<int>(mode), static_cast<int>(access), static_cast<int>(text), static_cast<int>(share));

  FILE * pFile = nullptr;

  if (mode == FileMode::Command)
  {
    MIKTEX_ASSERT(access == FileAccess::Read || access == FileAccess::Write);
    MIKTEX_ASSERT(!text);
    pFile = InitiateProcessPipe(path.ToString(), access, mode);
  }
  else
  {
    pFile = File::Open(path, mode, access, text, share);
  }

  try
  {
    OpenFileInfo info;
    info.pFile = pFile;
    info.fileName = path.ToString();
    info.mode = mode;
    info.access = access;
    openFilesMap.insert(pair<FILE *, OpenFileInfo>(pFile, info));
    if (setvbuf(pFile, 0, _IOFBF, 1024 * 4) != 0)
    {
      trace_error->WriteFormattedLine("core", T_("setvbuf() failed for some reason"));
    }
    RecordFileInfo(path, access);
    trace_files->WriteFormattedLine("core", "  => %p", pFile);
    return pFile;
  }
  catch (const exception &)
  {
    fclose(pFile);
    throw;
  }
}

FILE * SessionImpl::InitiateProcessPipe(const string & command, FileAccess access, FileMode & mode)
{
  Argv argv;
  argv.Build("", command);
  int argc = argv.GetArgc();
  if (argc == 0)
  {
    MIKTEX_FATAL_ERROR_2(T_("Invalid command."), "command", command);
  }
  string verb = argv[1];
  if (verb == "zcat" && argc == 3 && access == FileAccess::Read)
  {
    mode = FileMode::Open;
    return OpenFileOnStream(GzipStream::Create(argv[2], true));
  }
  else if (verb == "bzcat" && argc == 3 && access == FileAccess::Read)
  {
    mode = FileMode::Open;
    return OpenFileOnStream(BZip2Stream::Create(argv[2], true));
  }
  else if (verb == "xzcat" && argc == 3 && access == FileAccess::Read)
  {
    mode = FileMode::Open;
    return OpenFileOnStream(LzmaStream::Create(argv[2], true));
  }
  else
  {
    return POpen(command.c_str(), access == FileAccess::Read ? "r" : "w");
  }
}

MIKTEXSTATICFUNC(void) ReaderThread(unique_ptr<Stream> inStream, unique_ptr<Stream> outStream)
{
  try
  {
    char buf[PIPE_SIZE];
    int len;
    while ((len = inStream->Read(buf, PIPE_SIZE)) > 0)
    {
      outStream->Write(buf, len);
    }
  }
  catch (const exception &)
  {
  }
}

FILE * SessionImpl::OpenFileOnStream(std::unique_ptr<Stream> stream)
{
  array<unique_ptr<FileStream>, 2> files = CreatePipe(PIPE_SIZE);
  thread readerThread(&ReaderThread, move(stream), move(files[1]));
  readerThread.detach();
  return files[0]->Detach();
}

void SessionImpl::CloseFile(FILE * pFile)
{
  MIKTEX_ASSERT_BUFFER(pFile, sizeof(*pFile));
  trace_files->WriteFormattedLine("core", "CloseFile(%p)", pFile);
  map<const FILE *, OpenFileInfo>::iterator it = openFilesMap.find(pFile);
  bool isCommand = false;
  if (it != openFilesMap.end())
  {
    isCommand = (it->second.mode == FileMode::Command);
    openFilesMap.erase(it);
  }
  if (isCommand)
  {
    PClose(pFile);
  }
  else if (fclose(pFile) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fclose");
  }
}

bool SessionImpl::IsOutputFile(const FILE * pFile)
{
  MIKTEX_ASSERT(pFile != nullptr);
  map<const FILE *, OpenFileInfo>::const_iterator it = openFilesMap.find(pFile);
  if (it == openFilesMap.end())
  {
    return false;
  }
  return (it->second.mode != FileMode::Command && it->second.access == FileAccess::Write);
}

bool SessionImpl::StartFileInfoRecorder()
{
  recordingFileNames = true;
  return true;
}

bool SessionImpl::StartFileInfoRecorder(bool recordPackageNames)
{
  recordingFileNames = true;
  recordingPackageNames = recordPackageNames;
  return true;
}

void SessionImpl::SetRecorderPath(const PathName & path)
{
  if (!(recordingFileNames || recordingPackageNames))
  {
    return;
  }
  if (fileNameRecorderStream.IsOpen())
  {
    return;
  }
  fileNameRecorderStream.Attach(File::Open(path, FileMode::Create, FileAccess::Write));
  PathName cwd;
  cwd.SetToCurrentDirectory();
  fileNameRecorderStream.WriteFormattedLine("PWD %s", cwd.ToUnix().Get());
  vector<FileInfoRecord> fileInfoRecords = GetFileInfoRecords();
  for (vector<FileInfoRecord>::const_iterator it = fileInfoRecords.begin(); it != fileInfoRecords.end(); ++it)
  {
    fileNameRecorderStream.WriteFormattedLine("%s %s", it->access == FileAccess::Read ? "INPUT" : "OUTPUT", PathName(it->fileName).ToUnix().Get());
  }
  fileNameRecorderStream.Flush();
}

vector<FileInfoRecord> SessionImpl::GetFileInfoRecords()
{
  return fileInfoRecords;
}

void SessionImpl::CheckOpenFiles()
{
  for (map<const FILE *, OpenFileInfo>::const_iterator it = openFilesMap.begin(); it != openFilesMap.end(); ++it)
  {
    trace_error->WriteFormattedLine("core", T_("still open: %s"), Q_(it->second.fileName));
  }
}

void SessionImpl::WritePackageHistory()
{
  if (packageHistoryFile.empty())
  {
    return;
  }
  StreamWriter writer(File::Open(packageHistoryFile, FileMode::Append, FileAccess::Write));
  for (vector<FileInfoRecord>::const_iterator it = fileInfoRecords.begin(); it != fileInfoRecords.end(); ++it)
  {
    if (!it->packageName.empty())
    {
      writer.WriteLine(it->packageName);
    }
  }
  writer.Close();
}
