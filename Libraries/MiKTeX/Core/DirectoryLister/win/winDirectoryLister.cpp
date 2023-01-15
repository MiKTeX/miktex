/**
 * @file winDirectoryLister.cpp
 * @author Christian Schenk
 * @brief Directory lister implementation (Windows)
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <miktex/Core/DirectoryLister>
#include <miktex/Core/win/WindowsVersion>

#include "internal.h"

#include "winDirectoryLister.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName& directory)
{
    return make_unique<winDirectoryLister>(directory, nullptr, (int)Options::None);
}

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName& directory, const char* lpszPattern)
{
    return make_unique<winDirectoryLister>(directory, lpszPattern, (int)Options::None);
}

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName& directory, const char* lpszPattern, int options)
{
    return make_unique<winDirectoryLister>(directory, lpszPattern, options);
}

winDirectoryLister::winDirectoryLister(const PathName& directory, const char* lpszPattern, int options):
    directory(directory),
    pattern(lpszPattern == nullptr ? "" : lpszPattern),
    options(options)
{
}

winDirectoryLister::~winDirectoryLister()
{
    try
    {
        Close();
    }
    catch (const exception&)
    {
    }
}

void winDirectoryLister::Close()
{
    HANDLE handle = this->handle;
    if (handle == INVALID_HANDLE_VALUE)
    {
        return;
    }
    this->handle = INVALID_HANDLE_VALUE;
    if (!FindClose(handle))
    {
        MIKTEX_FATAL_WINDOWS_ERROR("FindClose");
    }
}

bool winDirectoryLister::GetNext(DirectoryEntry& direntry)
{
    DirectoryEntry2 direntry2;
    if (!GetNext(direntry2))
    {
        return false;
    }
    direntry.name = direntry2.name;
    direntry.wname = direntry2.wname;
    direntry.isDirectory = direntry2.isDirectory;
    return true;
}

inline bool IsDotDirectory(const wchar_t* lpszDirectory)
{
    if (lpszDirectory[0] != L'.')
    {
        return false;
    }
    if (lpszDirectory[1] == 0)
    {
        return true;
    }
    return lpszDirectory[1] == L'.' && lpszDirectory[2] == 0;
}

bool winDirectoryLister::GetNext(DirectoryEntry2& direntry2)
{
    WIN32_FIND_DATAW ffdat;
    do
    {
        if (handle == INVALID_HANDLE_VALUE)
        {
            PathName pathPattern(directory);
            if (pattern.empty())
            {
                pathPattern /= "*";
            }
            else
            {
                pathPattern /= pattern.c_str();
            }
            handle = FindFirstFileExW(
                pathPattern.ToExtendedLengthPathName().ToWideCharString().c_str(),
                FindExInfoBasic,
                &ffdat,
                (options & (int)Options::DirectoriesOnly) != 0 ? FindExSearchLimitToDirectories : FindExSearchNameMatch,
                nullptr,
                FIND_FIRST_EX_LARGE_FETCH);
            if (handle == INVALID_HANDLE_VALUE)
            {
                if (::GetLastError() != ERROR_FILE_NOT_FOUND)
                {
                    MIKTEX_FATAL_WINDOWS_ERROR_2("FindFirstFileExW", "directory", directory.ToString());
                }
                return false;
            }
        }
        else
        {
            if (!FindNextFileW(handle, &ffdat))
            {
                if (::GetLastError() != ERROR_NO_MORE_FILES)
                {
                    MIKTEX_FATAL_WINDOWS_ERROR_2("FindNextFileExW", "directory", directory.ToString());
                }
                return false;
            }
        }
    } while ((((ffdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) && IsDotDirectory(ffdat.cFileName) && ((options & (int)Options::IncludeDotAndDotDot) == 0))
        || (((options & (int)Options::DirectoriesOnly) != 0) && ((ffdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
        || (((options & (int)Options::FilesOnly) != 0) && ((ffdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)));
    direntry2.wname = ffdat.cFileName;
    direntry2.name = WU_(ffdat.cFileName);
    direntry2.isDirectory = (ffdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    direntry2.size = ffdat.nFileSizeLow;
    // TODO: large file support
    if (ffdat.nFileSizeHigh != 0)
    {
        MIKTEX_UNEXPECTED();
    }
    return true;
}
