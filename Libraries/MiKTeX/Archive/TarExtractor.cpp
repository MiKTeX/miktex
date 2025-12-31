/**
 * @file TarExtractor.cpp
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief TarExtractor implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2025 Christian Schenk
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

#include "TarExtractor.h"

using namespace std;

using namespace MiKTeX::Archive;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

const size_t BLOCKSIZE = 512;

struct Header
{

public:

    Header()
    {
        MIKTEX_ASSERT(sizeof(*this) == BLOCKSIZE);
    }

    bool Check() const
    {
        unsigned myHeaderCheckSum = 0;
        size_t i;
        for (i = 0; i < offsetof(Header, chksum); ++i)
        {
            myHeaderCheckSum += reinterpret_cast<const unsigned char*>(this)[i];
        }
        myHeaderCheckSum += ' ' * sizeof(chksum);
        i += sizeof(chksum);
        for (; i < sizeof(*this); ++i)
        {
            myHeaderCheckSum += reinterpret_cast<const unsigned char*>(this)[i];
        }
        return myHeaderCheckSum == GetHeaderCheckSum();
    }

    bool IsEndOfArchive() const
    {
        for (size_t i = 0; i < sizeof(*this); ++i)
        {
            if (reinterpret_cast<const unsigned char*>(this)[i] != 0)
            {
                return false;
            }
        }
        return true;
    }

    PathName GetFileName() const
    {
        PathName ret;
        if (IsUSTAR())
        {
            ret = prefix;
        }
        ret /= GetString(name);
        return ret;
    }

    unsigned long GetFileMode() const
    {
        return GetOctal(mode);
    }

    unsigned GetUid() const
    {
        return GetOctal(uid);
    }

    unsigned GetGid() const
    {
        return GetOctal(gid);
    }

    size_t GetFileSize() const
    {
        return GetOctal(size);
    }

    time_t GetLastModificationTime() const
    {
        return GetOctal(mtime);
    }

    unsigned long GetHeaderCheckSum() const
    {
        return GetOctal(chksum);
    }

    enum Type
    {
        RegularFile = '0',
        AlternateRegularFile = '\0',
        Link = '1',
        SymbolicLink = '2',
        CharacterSpecial = '3',
        BlockSpecial = '4',
        Directory = '5',
        FIFOSpecial = '6',
        Reserved = '7',
        LongName = 'L'
    };

    Type GetType() const
    {
        return static_cast<Type>(typeflag[0]);
    }

    bool IsNormalFile() const
    {
        return GetType() == RegularFile || GetType() == AlternateRegularFile;
    }

    string GetLinkName() const
    {
        return GetString(linkname);
    }

    bool IsUSTAR() const
    {
        return memcmp(magic, "ustar", 5) == 0 && (magic[5] == 0 || magic[5] == ' ');
    }

    unsigned GetVersion() const
    {
        return GetOctal(version);
    }

    string GetOwner() const
    {
        return GetString(uname);
    }

    string GetGroup() const
    {
        return GetString(gname);
    }

    unsigned GetDevMajor() const
    {
        return GetOctal(devmajor);
    }

    unsigned GetDevMinor() const
    {
        return GetOctal(devminor);
    }

private:

    template<size_t N> string GetString(const char(&field)[N]) const
    {
        char fieldz[N + 1];
        memcpy(fieldz, field, N);
        fieldz[N] = 0;
        return fieldz;
    }

    template<size_t N> int GetOctal(const char(&field)[N]) const
    {
        return std::stoi(GetString(field), nullptr, 8);
    }

    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char reserved[12];
};

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

            if (header.IsEndOfArchive())
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

            PathName dest = header.GetFileName();
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
