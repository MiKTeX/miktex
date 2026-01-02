/**
 * @file TarCreator.cpp
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief TarCreator implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#include "config.h"

#include <miktex/Core/FileStream>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>

#include "internal.h"
#include "tar.h"

#include "TarCreator.h"

using namespace std;

using namespace MiKTeX::Archive;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

TarCreator::TarCreator()
{
}

TarCreator::~TarCreator()
{
}

void TarCreator::Create(const PathName& path, const vector<FileSet>& fileSets)
{
    FileStream stream(File::Open(path, FileMode::Create, FileAccess::Write, false));
    Create(&stream, fileSets);
    stream.Close();
}

void TarCreator::Create(Stream* stream, const vector<FileSet>& fileSets)
{
    for (const FileSet& fileSet : fileSets)
    {
        for (const string& name : fileSet.names)
        {
            auto path = fileSet.baseDir / name;
            if (Directory::Exists(path))
            {
                WriteDirectory(stream, path, name);
                continue;
            }
            WriteFile(stream, path, name);
        }
    }
    char zeroBlock[BLOCKSIZE] = {};
    stream->Write(zeroBlock, sizeof(zeroBlock));
    stream->Write(zeroBlock, sizeof(zeroBlock));
}

void TarCreator::WriteDirectory(Stream* stream, const PathName& path, const string& name)
{
    Header header;
    header.SetFileName(name + "/");
    header.SetType(Header::Type::Directory);
    header.SetLastModificationTime(File::GetLastWriteTime(path));
    header.SetChecksum();
    stream->Write(&header, sizeof(Header));
    auto lister = DirectoryLister::Open(path);
    DirectoryEntry dirEntry;
    while (lister->GetNext(dirEntry))
    {
        if (dirEntry.isDirectory)
        {
            WriteDirectory(stream, path / dirEntry.name, name + "/" + dirEntry.name);
            continue;
        }
        WriteFile(stream, path / dirEntry.name, name + "/" + dirEntry.name);
    }
    lister->Close();
}

void TarCreator::WriteFile(Stream* stream, const PathName& path, const string& name)
{
    auto fileSize = File::GetSize(path);
    Header header;
    header.SetFileName(name);
    header.SetType(Header::Type::RegularFile);
    header.SetFileSize(fileSize);
    header.SetLastModificationTime(File::GetLastWriteTime(path));
    header.SetChecksum();
    stream->Write(&header, sizeof(Header));
    FileStream fileStream(File::Open(path, FileMode::Open, FileAccess::Read, false));
    const size_t BUFSIZE = 4096;
    char buffer[BUFSIZE];
    size_t bytesRemaining = fileSize;
    while (bytesRemaining > 0)
    {
        size_t n = (bytesRemaining > BUFSIZE ? BUFSIZE : bytesRemaining);
        size_t bytesRead = fileStream.Read(buffer, n);
        if (bytesRead != n)
        {
            MIKTEX_UNEXPECTED();
        }
        stream->Write(buffer, n);
        bytesRemaining -= n;
    }
    size_t paddingSize = (BLOCKSIZE - fileSize % BLOCKSIZE) % BLOCKSIZE;
    if (paddingSize > 0)
    {
        char zeroBlock[BLOCKSIZE] = {};
        stream->Write(zeroBlock, paddingSize);
    }
    fileStream.Close();
}

