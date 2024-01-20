/**
 * @file miktex/Util/PathNameUtil.h
 * @author Christian Schenk
 * @brief PathNameUtil class
 *
 * @copyright Copyright © 2020-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <miktex/Util/config.h>

#include <string>

MIKTEX_UTIL_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE PathNameUtil
{

public:

    PathNameUtil() = delete;
    PathNameUtil(const PathNameUtil& other) = delete;
    PathNameUtil& operator=(const PathNameUtil& other) = delete;
    PathNameUtil(PathNameUtil&& other) = delete;
    PathNameUtil& operator=(PathNameUtil&& other) = delete;
    ~PathNameUtil() = delete;

    static constexpr char DosDirectoryDelimiter{ '\\' };
    static constexpr char UnixDirectoryDelimiter{ '/' };
    static constexpr char DosPathNameDelimiter{ ';' };
    static constexpr char UnixPathNameDelimiter{ ':' };
    static constexpr char DosVolumeDelimiter{ ':' };

#if defined(MIKTEX_WINDOWS)
    static constexpr char AltDirectoryDelimiter{ UnixDirectoryDelimiter };
#endif

#if defined(MIKTEX_WINDOWS)
    static constexpr char DirectoryDelimiter{ DosDirectoryDelimiter };
#elif defined(MIKTEX_UNIX)
    static constexpr char DirectoryDelimiter{ UnixDirectoryDelimiter };
#endif

#if defined(MIKTEX_WINDOWS)
    static constexpr char PathNameDelimiter{ DosPathNameDelimiter };
#elif defined(MIKTEX_UNIX)
    static constexpr char PathNameDelimiter{ UnixPathNameDelimiter };
#endif

#if defined(MIKTEX_WINDOWS)
    static constexpr char VolumeDelimiter{ DosVolumeDelimiter };
#endif

    static bool IsDosDriveLetter(int ch)
    {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    }

    static bool IsDosVolumeDelimiter(int ch)
    {
        return ch == DosVolumeDelimiter;
    }

    /// Tests if a character is a directory delimiter.
    /// @param ch The character to test.
    /// @return Returns true if the character is a directory delimiter.
    static bool IsDirectoryDelimiter(int ch)
    {
        if (ch == DirectoryDelimiter)
        {
            return true;
        }
#if defined(MIKTEX_WINDOWS)
        return ch == AltDirectoryDelimiter;
#else
        return false;
#endif
    }

    static bool IsExplicitlyRelative(const std::string& path)
    {
        if (path.empty())
        {
            return false;
        }
        else if (path[0] == '.')
        {
            return (path.length() > 1 && IsDirectoryDelimiter(path[1])) || (path.length() > 2 && path[1] == '.' && IsDirectoryDelimiter(path[2]));
        }
        else
        {
            return false;
        }
    }

    static bool IsAbsolutePath(const std::string& path)
    {
        if (path.empty())
        {
            return false;
        }
        else if (IsDirectoryDelimiter(path[0]))
        {
            // "/xyz/foo.txt", also "//server/xyz/foo.txt"
            return true;
        }
#if defined(MIKTEX_WINDOWS)
        else if (path.length() > 2 && PathNameUtil::IsDosDriveLetter(path[0]) // "C:\xyz\foo.txt"
            && IsDosVolumeDelimiter(path[1])
            && IsDirectoryDelimiter(path[2]))
        {
            return true;
        }
#endif
        else
        {
            return false;
        }
    }

    static bool IsFullyQualifiedPath(const std::string& path)
    {
#if defined(MIKTEX_WINDOWS)
        if (path.length() < 3)
        {
            return false;
        }
        else if (IsDirectoryDelimiter(path[0]) && IsDirectoryDelimiter(path[1]))
        {
            // \\server\xyz\foo.txt
            return true;
        }
        else if (PathNameUtil::IsDosDriveLetter(path[0])
            && IsDosVolumeDelimiter(path[1])
            && IsDirectoryDelimiter(path[2]))
        {
            // C:\xyz\foo.txt
            return true;
        }
        else
        {
            return false;
        }
#else
        return IsAbsolutePath(path);
#endif
    }

    static char ToUnix(char ch)
    {
        return ch == DosDirectoryDelimiter ? UnixDirectoryDelimiter : ch;
    }

    static char ToDos(char ch)
    {
        return ch == UnixDirectoryDelimiter ? DosDirectoryDelimiter : ch;
    }

    static void ConvertToUnix(std::string& path)
    {
        for (char& ch : path)
        {
            ch = ToUnix(ch);
        }
    }

    static void ConvertToDos(std::string& path)
    {
        for (char& ch : path)
        {
            ch = ToDos(ch);
        }
    }

#if defined(MIKTEX_WINDOWS)
    static MIKTEXUTILCEEAPI(std::wstring) ToLengthExtendedPathName(const std::string& path);
#endif
};

MIKTEX_UTIL_END_NAMESPACE;
