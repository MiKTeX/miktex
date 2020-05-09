/* files.cpp: file system operations

   Copyright (C) 1996-2020 Christian Schenk

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

#include <fcntl.h>

#include <fstream>
#include <thread>

#include <fmt/format.h>
#include <fmt/ostream.h>

#if defined(_MSC_VER)
#  include <io.h>
#endif

#if defined(MIKTEX_UNIX)
#  include <unistd.h>
#endif

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/BZip2Stream>
#include <miktex/Core/GzipStream>
#include <miktex/Core/FileStream>
#include <miktex/Core/LzmaStream>
#include <miktex/Core/PathName>

#include "internal.h"

#include "Fndb/FileNameDatabase.h"
#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;

const size_t PIPE_SIZE = 4096;

MIKTEXSTATICFUNC(int) Close(int fd)
{
#if defined(_MSC_VER)
  return _close(fd);
#else
  return close(fd);
#endif
}

MIKTEXSTATICFUNC(FILE*) POpen(const char* command, const char* mode)
{
  FILE* file;
#if defined(_MSC_VER)
  file = _wpopen(UW_(command), UW_(mode));
#else
  file = popen(command, mode);
#endif
  if (file == nullptr)
  {
    MIKTEX_FATAL_CRT_ERROR_2("popen", "command", command, "mode", mode);
  }
  return file;
}

MIKTEXSTATICFUNC(int) PClose(FILE* file)
{
  int exitCode;
#if defined(_MSC_VER)
  exitCode = _pclose(file);
#else
  exitCode = pclose(file);
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
#if defined(_MSC_VER)
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
  catch (const exception&)
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

void SessionImpl::RecordFileInfo(const PathName& path, FileAccess access)
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
        vector<Fndb::Record> records;
        if (fndb->Search(pathRelPath, MPM_ROOT_PATH, false, records))
        {
          fir.packageName = records[0].fileNameInfo;
        }
      }
    }
  }
  fileInfoRecords.push_back(fir);
  if (fileNameRecorderStream.is_open())
  {
    fileNameRecorderStream << (fir.access == FileAccess::Read ? "INPUT" : "OUTPUT") << " " << PathName(fir.fileName).ToUnix() << std::endl;
  }
}

FILE* SessionImpl::TryOpenFile(const PathName& path, FileMode mode, FileAccess access, bool text)
{
  try
  {
    return OpenFile(path, mode, access, text);
  }
#if defined(MIKTEX_WINDOWS)
  catch (const SharingViolationException&)
  {
    return nullptr;
  }
#endif
  catch (const UnauthorizedAccessException&)
  {
    return nullptr;
  }
  catch (const FileNotFoundException&)
  {
    return nullptr;
  }
}

FILE* SessionImpl::OpenFile(const PathName& path, FileMode mode, FileAccess access, bool text)
{
  trace_files->WriteLine("core", fmt::format("OpenFile(\"{0}\", {1}, {2:x}, {3})", path, static_cast<int>(mode), static_cast<int>(access), text));

  FILE* file = nullptr;

  if (mode == FileMode::Command)
  {
    MIKTEX_ASSERT(access == FileAccess::Read || access == FileAccess::Write);
    MIKTEX_ASSERT(!text);
    file = InitiateProcessPipe(path.ToString(), access, mode);
  }
  else
  {
    file = File::Open(path, mode, access, text);
  }

  try
  {
    OpenFileInfo info;
    info.file = file;
    info.fileName = path.ToString();
    info.mode = mode;
    info.access = access;
    openFilesMap.insert(pair<FILE*, OpenFileInfo>(file, info));
    if (setvbuf(file, 0, _IOFBF, 1024 * 4) != 0)
    {
      trace_error->WriteLine("core", TraceLevel::Error, "setvbuf() failed for some reason");
    }
    RecordFileInfo(path, access);
    trace_files->WriteLine("core", fmt::format("  => {0}", static_cast<void*>(file)));
    return file;
  }
  catch (const exception&)
  {
    if (mode == FileMode::Command)
    {
      PClose(file);
    }
    else
    {
      fclose(file);
    }
    throw;
  }
}

FILE* SessionImpl::InitiateProcessPipe(const string& command, FileAccess access, FileMode& mode)
{
  return POpen(command.c_str(), access == FileAccess::Read ? "r" : "w");
}

MIKTEXSTATICFUNC(void) ReaderThread(unique_ptr<Stream> inStream, unique_ptr<Stream> outStream)
{
  try
  {
    char buf[PIPE_SIZE];
    size_t len;
    while ((len = inStream->Read(buf, PIPE_SIZE)) > 0)
    {
      outStream->Write(buf, len);
    }
  }
  catch (const exception&)
  {
  }
}

FILE* SessionImpl::OpenFileOnStream(std::unique_ptr<Stream> stream)
{
  array<unique_ptr<FileStream>, 2> files = CreatePipe(PIPE_SIZE);
  thread readerThread(&ReaderThread, move(stream), move(files[1]));
  readerThread.detach();
  return files[0]->Detach();
}

pair<bool, Session::OpenFileInfo> SessionImpl::TryGetOpenFileInfo(FILE* file)
{
  map<const FILE*, OpenFileInfo>::const_iterator it = openFilesMap.find(file);
  if (it == openFilesMap.end())
  {
    return make_pair<bool, Session::OpenFileInfo>(false, Session::OpenFileInfo());
  }
  else
  {
    return make_pair<bool, Session::OpenFileInfo>(true, Session::OpenFileInfo(it->second));
  }
}

void SessionImpl::CloseFile(FILE* file)
{
  MIKTEX_ASSERT_BUFFER(file, sizeof(*file));
  trace_files->WriteLine("core", fmt::format("CloseFile({0})", static_cast<void*>(file)));
  map<const FILE*, OpenFileInfo>::iterator it = openFilesMap.find(file);
  bool isCommand = false;
  if (it != openFilesMap.end())
  {
    isCommand = (it->second.mode == FileMode::Command);
    openFilesMap.erase(it);
  }
  if (isCommand)
  {
    PClose(file);
  }
  else if (fclose(file) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fclose");
  }
}

bool SessionImpl::IsOutputFile(const FILE* file)
{
  MIKTEX_ASSERT(file != nullptr);
  map<const FILE*, OpenFileInfo>::const_iterator it = openFilesMap.find(file);
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

void SessionImpl::SetRecorderPath(const PathName& path)
{
  if (!(recordingFileNames || recordingPackageNames))
  {
    return;
  }
  if (fileNameRecorderStream.is_open())
  {
    return;
  }
  fileNameRecorderStream.open(path.ToNativeString());
  if (!fileNameRecorderStream.is_open())
  {
    MIKTEX_FATAL_CRT_ERROR_2("ofsteam::open", "path", path.ToString());
  }
  fileNameRecorderStream.exceptions(ifstream::badbit | ifstream::failbit);
  PathName cwd;
  cwd.SetToCurrentDirectory();
  fileNameRecorderStream << "PWD " << cwd.ToUnix() << "\n";
  vector<FileInfoRecord> fileInfoRecords = GetFileInfoRecords();
  for (vector<FileInfoRecord>::const_iterator it = fileInfoRecords.begin(); it != fileInfoRecords.end(); ++it)
  {
    fileNameRecorderStream << (it->access == FileAccess::Read ? "INPUT" : "OUTPUT") << " " << PathName(it->fileName).ToUnix() << "\n";
  }
  fileNameRecorderStream.flush();
}

vector<FileInfoRecord> SessionImpl::GetFileInfoRecords()
{
  return fileInfoRecords;
}

void SessionImpl::CheckOpenFiles()
{
  for (map<const FILE*, OpenFileInfo>::const_iterator it = openFilesMap.begin(); it != openFilesMap.end(); ++it)
  {
    trace_error->WriteLine("core", TraceLevel::Error, fmt::format("still open: {0}", Q_(it->second.fileName)));
  }
}

void SessionImpl::WritePackageHistory()
{
  if (packageHistoryFile.empty())
  {
    return;
  }
  ofstream stream = File::CreateOutputStream(PathName(packageHistoryFile), ios_base::app);
  for (vector<FileInfoRecord>::const_iterator it = fileInfoRecords.begin(); it != fileInfoRecords.end(); ++it)
  {
    if (!it->packageName.empty())
    {
      stream << it->packageName << "\n";
    }
  }
  stream.close();
}
