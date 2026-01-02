/**
 * @file TarExtractor.cpp
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief TarExtractor implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Debug>
#include <miktex/Core/Directory>
#include <miktex/Core/FileStream>
#include <miktex/Trace/Trace>

#include "internal.h"
#include "tar.h"

#include "TarExtractor.h"

using namespace std;

using namespace MiKTeX::Archive;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

void TarExtractor::ReadBlock(void* data)
{
    size_t n = streamIn->Read(data, BLOCKSIZE);
    totalBytesRead += n;
    if (n != BLOCKSIZE)
    {
        MIKTEX_UNEXPECTED();
    }
}

void TarExtractor::Skip(size_t numBytes)
{
    const size_t BUFSIZE = 4096;
    char buffer[BUFSIZE];
    size_t bytesRead = 0;
    while (bytesRead < numBytes)
    {
        size_t remaining = numBytes - bytesRead;
        size_t n = (remaining > BUFSIZE ? BUFSIZE : remaining);
        if (Read(buffer, n) != n)
        {
            MIKTEX_UNEXPECTED();
        }
        bytesRead += n;
    }
}

TarExtractor::TarExtractor() :
    traceStream(TraceStream::Open(MIKTEX_TRACE_EXTRACTOR)),
    traceStopWatch(TraceStream::Open(MIKTEX_TRACE_STOPWATCH)),
    haveLongName(false)
{
}

TarExtractor::~TarExtractor()
{
}

void TarExtractor::Extract(Stream* streamIn_, const PathName& destDir, bool makeDirectories, IExtractCallback* callback, const string& prefix)
{
    try
    {
        streamIn = streamIn_;
        totalBytesRead = 0;

        traceStream->WriteLine(TRACE_FACILITY, fmt::format(T_("extracting to {0} ({1})"), Q_(destDir), (makeDirectories ? T_("make directories") : T_("don't make directories"))));

        size_t len;
        Header header;
        size_t prefixLen = prefix.length();
        unsigned fileCount = 0;

        bool checkHeader = true;

        CharBuffer<char> buffer;
        buffer.Reserve(1024 * 1024);

        while ((len = Read(&header, sizeof(header))) > 0)
        {
            // read next header
            if (len != sizeof(header))
            {
                MIKTEX_UNEXPECTED();
            }

            if (header.IsZero())
            {
                break;
            }

            if (checkHeader)
            {
                if (!header.Check())
                {
                    MIKTEX_UNEXPECTED();
                }
#if !defined(MIKTEX_DEBUG)
                checkHeader = false;
#endif
            }

            PathName dest = PathName(header.GetFileName());
            size_t size = header.GetFileSize();

            if (!header.IsNormalFile())
            {
                if (header.GetType() == Header::LongName)
                {
                    if (size >= BLOCKSIZE)
                    {
                        MIKTEX_UNEXPECTED();
                    }
                    char longNameData[BLOCKSIZE];
                    ReadBlock(longNameData);
                    longNameData[size] = 0;
                    longName = longNameData;
                    haveLongName = true;
                }
                else
                {
                    Skip(((size + sizeof(Header) - 1) / sizeof(Header)) * sizeof(Header));
                }
                continue;
            }

            if (haveLongName)
            {
                dest = longName;
                haveLongName = false;
            }

            // skip directory prefix
            if (PathName::ComparePrefixes(PathName(prefix), dest, prefixLen) == 0)
            {
                PathName tmp(dest);
                dest = tmp.GetData() + prefixLen;
            }

            // make the destination path name
            PathName path(destDir);
            if (!makeDirectories)
            {
                dest.RemoveDirectorySpec();
            }
            path /= dest.ToString();

            // notify the client
            if (callback != nullptr)
            {
                callback->OnBeginFileExtraction(path.ToString(), size);
            }

            // create the destination directory
            Directory::Create(PathName(path).RemoveFileSpec());

            // remove the existing file
            if (File::Exists(path))
            {
                File::Delete(path, { FileDeleteOption::TryHard });
            }

            // extract the file
            FileStream streamOut(File::Open(path, FileMode::Create, FileAccess::Write, false));
            size_t bytesRead = 0;
            while (bytesRead < size)
            {
                size_t remaining = size - bytesRead;
                size_t n = (remaining > buffer.GetCapacity() ? buffer.GetCapacity() : remaining);
                if (Read(buffer.GetData(), n) != n)
                {
                    MIKTEX_UNEXPECTED();
                }
                streamOut.Write(buffer.GetData(), n);
                bytesRead += n;
            }
            // set time when the file was created
            time_t time = header.GetLastModificationTime();
            File::SetTimes(streamOut.GetFile(), time, time, time);
            streamOut.Close();

            // skip extra bytes
            if (bytesRead % sizeof(Header) > 0)
            {
                Skip(sizeof(Header) - bytesRead % sizeof(Header));
            }

            fileCount += 1;

#if 0
            // set file attributes
            File::SetAttributes(path, todo);
#endif

            // notify the client
            if (callback != nullptr)
            {
                callback->OnEndFileExtraction("", size);
            }
        }

        traceStream->WriteLine(TRACE_FACILITY, fmt::format(T_("extracted {0} file(s)"), fileCount));
    }
    catch (const exception&)
    {
        traceStream->WriteLine(TRACE_FACILITY, fmt::format(T_("{0} bytes were read from the tar stream"), totalBytesRead));
        throw;
    }
}

void TarExtractor::Extract(const PathName& path, const PathName& destDir, bool makeDirectories, IExtractCallback* callback, const string& prefix)
{
    traceStream->WriteLine(TRACE_FACILITY, fmt::format(T_("extracting {0}"), Q_(path)));
    FileStream stream(File::Open(path, FileMode::Open, FileAccess::Read, false));
    Extract(&stream, destDir, makeDirectories, callback, prefix);
    stream.Close();
}
