/**
 * @file PathName.cpp
 * @author Christian Schenk
 * @brief PathName class
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#if defined(MIKTEX_WINDOWS)
#include <direct.h>
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/PathName.h"
#include "miktex/Util/PathNameParser.h"

#include "internal.h"

constexpr auto CURRENT_DIRECTORY = ".";
constexpr auto PARENT_DIRECTORY = "..";

using namespace std;

using namespace MiKTeX::Util;

int PathName::Compare(const PathName& path1_, const PathName& path2_)
{
    PathName path1(path1_);
    PathName path2(path2_);
#if defined(MIKTEX_WINDOWS)
    path1.TransformForComparison();
    path2.TransformForComparison();
#endif
    const char* lpszPath1 = path1.GetData();
    const char* lpszPath2 = path2.GetData();

    int ret;
    int cmp;

    while ((cmp = *lpszPath1 - *lpszPath2) == 0 && *lpszPath1 != 0)
    {
        ++lpszPath1;
        ++lpszPath2;
    }

    if (cmp != 0)
    {
        if (
            (*lpszPath1 == 0 && PathNameUtil::IsDirectoryDelimiter(*lpszPath2) && *(lpszPath2 + 1) == 0)
            || (*lpszPath2 == 0 && PathNameUtil::IsDirectoryDelimiter(*lpszPath1) && *(lpszPath1 + 1) == 0))
        {
            return 0;
        }
    }

    if (cmp < 0)
    {
        ret = -1;
    }
    else if (cmp > 0)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

int PathName::ComparePrefixes(const PathName& path1_, const PathName& path2_, size_t count)
{
    if (count == 0)
    {
        return 0;
    }

    PathName path1(path1_);
    PathName path2(path2_);
#if defined(MIKTEX_WINDOWS)
    path1.TransformForComparison();
    path2.TransformForComparison();
#endif
    const char* lpszPath1 = path1.GetData();
    const char* lpszPath2 = path2.GetData();

    for (size_t i = 0; i < count; ++i, ++lpszPath1, ++lpszPath2)
    {
        if (*lpszPath1 == 0 || *lpszPath1 != *lpszPath2)
        {
            return *reinterpret_cast<const unsigned char*>(lpszPath1) - *reinterpret_cast<const unsigned char*>(lpszPath2);
        }
    }

    return 0;
}

PathName GetFullyQualifiedPath(const char* lpszPath)
{
    PathName path;

    if (!PathNameUtil::IsFullyQualifiedPath(lpszPath))
    {
#if defined(MIKTEX_WINDOWS)
        if (PathNameUtil::IsDosDriveLetter(lpszPath[0]) && PathNameUtil::IsDosVolumeDelimiter(lpszPath[1]) && lpszPath[2] == 0)
        {
            path = lpszPath;
            path += PathNameUtil::DirectoryDelimiter;
            return path;
        }
        if (PathNameUtil::IsDirectoryDelimiter(lpszPath[0]))
        {
            int currentDrive = _getdrive();
            if (currentDrive == 0)
            {
                // TODO
                throw Exception("unexpected");
            }
            // EXPECT: currentDrive >= 1 && currentDrive <= 26
            char currentDriveLetter = 'A' + currentDrive - 1;
            path = fmt::format("{0}{1}", currentDriveLetter, PathNameUtil::DosVolumeDelimiter);
        }
        else
        {
            path.SetToCurrentDirectory();
        }
#else
        path.SetToCurrentDirectory();
#endif
    }

    PathName fixme(lpszPath);
    for (PathNameParser parser(fixme); parser; ++parser)
    {
        if (PathName::Equals(PathName(*parser), PathName(PARENT_DIRECTORY)))
        {
            path.CutOffLastComponent();
        }
        else if (!PathName::Equals(PathName(*parser), PathName(CURRENT_DIRECTORY)))
        {
            path /= *parser;
        }
    }

    return path;
}

PathName& PathName::Convert(ConvertPathNameOptions options)
{
    bool toUnix = options[ConvertPathNameOption::ToUnix];
    bool toDos = options[ConvertPathNameOption::ToDos];

    bool toUpper = options[ConvertPathNameOption::MakeUpper];
    bool toLower = options[ConvertPathNameOption::MakeLower];

    bool makeFQ = options[ConvertPathNameOption::MakeFullyQualified];

#if defined(MIKTEX_WINDOWS)
    bool toExtendedLengthPathName = options[ConvertPathNameOption::ToExtendedLengthPathName];
#endif

    if (makeFQ)
    {
        *this = GetFullyQualifiedPath(GetData());
    }

#if defined(MIKTEX_WINDOWS)
    if (toExtendedLengthPathName)
    {
        *this = PathNameUtil::ToLengthExtendedPathName(ToString());
    }
#endif

    if (options[ConvertPathNameOption::CleanUp])
    {
        vector<string> components = Split(*this);
        for (auto it = components.begin(); it != components.end();)
        {
            if (*it == CURRENT_DIRECTORY)
            {
                it = components.erase(it);
            }
            else if (*it == PARENT_DIRECTORY)
            {
                if (it != components.begin() && *(it - 1) != PARENT_DIRECTORY)
                {
                    it = components.erase(--it);
                    it = components.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            else
            {
                ++it;
            }
        }
        *this = "";
        for (auto it = components.begin(); it != components.end(); ++it)
        {
            *this /= *it;
        }
    }

    if (options[ConvertPathNameOption::Canonicalize])
    {
        Helpers::CanonicalizePathName(*this);
    }

    if (toUnix)
    {
        string s = this->ToString();
        PathNameUtil::ConvertToUnix(s);
        *this = s;
    }
    else if (toDos)
    {
        string s = this->ToString();
        PathNameUtil::ConvertToDos(s);
        *this = s;
    }

    if (toUpper || toLower)
    {
        if (Helpers::IsPureAscii(GetData()))
        {
            for (char* lpsz = GetData(); *lpsz != 0; ++lpsz)
            {
                *lpsz = toUpper ? Helpers::ToUpperAscii(*lpsz) : Helpers::ToLowerAscii(*lpsz);
            }
        }
        else
        {
            CharBuffer<wchar_t> wideCharBuffer(GetData());
            locale defaultLocale("");
            for (wchar_t* lpsz = wideCharBuffer.GetData(); *lpsz != 0; ++lpsz)
            {
                *lpsz = toUpper ? Helpers::ToUpper(*lpsz, defaultLocale) : Helpers::ToLower(*lpsz, defaultLocale);
            }
            *this = wideCharBuffer.GetData();
        }
    }

    return *this;
}

static bool InternalMatch(const char* lpszPattern, const char* lpszPath)
{
    switch (*lpszPattern)
    {
    case 0:
        return *lpszPath == 0;

    case '*':
        // RECURSION
        return InternalMatch(lpszPattern + 1, lpszPath) || (*lpszPath != 0 && InternalMatch(lpszPattern, lpszPath + 1));

    case '?':
        // RECURSION
        return *lpszPath != 0 && InternalMatch(lpszPattern + 1, lpszPath + 1);

    default:
        // RECURSION
        return *lpszPattern == *lpszPath && InternalMatch(lpszPattern + 1, lpszPath + 1);
    }
}

bool PathName::Match(const std::string& pattern, const PathName& path_)
{
    PathName path(path_);
    return InternalMatch(PathName(pattern).TransformForComparison().GetData(), path.TransformForComparison().GetData());
}

vector<string> PathName::Split(const PathName& path)
{
    vector<string> result;
    for (PathNameParser parser(path); parser; ++parser)
    {
        result.push_back(*parser);
    }
    return result;
}

void PathName::Split(const PathName& path, string& directory, string& fileNameWithoutExtension, string& extension)
{
    const char* lpsz;

    const char* lpszName_ = nullptr;

    // find the beginning of the name
    for (lpsz = path.GetData(); *lpsz != 0; ++lpsz)
    {
        if (PathNameUtil::IsDirectoryDelimiter(*lpsz))
        {
            lpszName_ = lpsz + 1;
        }
    }
    if (lpszName_ == nullptr)
    {
        lpszName_ = path.GetData();
    }

    directory.assign(path.GetData(), lpszName_ - path.GetData());

    // find the extension
    const char* lpszExtension_ = nullptr;
    for (lpsz = lpszName_; *lpsz != 0; ++lpsz)
    {
        if (*lpsz == '.')
        {
            lpszExtension_ = lpsz;
        }
    }
    if (lpszExtension_ == nullptr)
    {
        lpszExtension_ = lpsz;
    }

    fileNameWithoutExtension.assign(lpszName_, lpszExtension_ - lpszName_);

    extension = lpszExtension_;
}

string PathName::GetExtension() const
{
    const char* e = Helpers::GetFileNameExtension(GetData());
    return e == nullptr ? string() : string(e);
}

PathName& PathName::SetExtension(const std::string& extension, bool override)
{
    string directory;
    string fileNameWithoutExtension;
    string oldExtension;

    Split(*this, directory, fileNameWithoutExtension, oldExtension);

    if (oldExtension.empty() || override)
    {
        *this = directory;
        AppendComponent(fileNameWithoutExtension);
        if (!extension.empty())
        {
            size_t n = GetLength();
            if (extension[0] != '.')
            {
                if (n + 1 >= GetCapacity())
                {
                    throw Unexpected("buf too small");
                }
                (*this)[n] = '.';
                ++n;
            }
            n += StringUtil::CopyCeeString(&(*this)[n], GetCapacity() - n, extension.c_str());
        }
    }

    return *this;
}

PathName& PathName::AppendDirectoryDelimiter()
{
    size_t l = GetLength();
    if (l == 0 || !PathNameUtil::IsDirectoryDelimiter(Base::operator[](l - 1)))
    {
        Base::Append(PathNameUtil::DirectoryDelimiter);
    }
    return *this;
}

// TODO: code review
PathName& PathName::CutOffLastComponent(bool allowSelfCutting)
{
    Helpers::RemoveDirectoryDelimiter(GetData());
    bool noCut = true;
    for (size_t end = GetLength(); noCut && end > 0; --end)
    {
        if (end > 0 && PathNameUtil::IsDirectoryDelimiter(Base::operator[](end - 1)))
        {
#if defined(MIKTEX_WINDOWS)
            if (end > 1 && Base::operator[](end - 2) == PathNameUtil::DosVolumeDelimiter)
            {
                Base::operator[](end) = 0;
            }
            else
#endif
                if (end == 1)
                {
                    Base::operator[](1) = 0;
                }
                else
                {
                    while (end > 0 && PathNameUtil::IsDirectoryDelimiter(Base::operator[](end - 1)))
                    {
                        --end;
                        Base::operator[](end) = 0;
                    }
                }
            noCut = false;
        }
    }
    if (noCut && allowSelfCutting)
    {
        Base::operator[](0) = 0;
    }
    return *this;
}

size_t PathName::GetHash() const
{
    // see http://www.isthe.com/chongo/tech/comp/fnv/index.html
#if defined(_M_AMD64) || defined(_M_X64) || defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(__arch64__)
    const size_t FNV_prime = 1099511628211;
    const size_t offset_basis = 14695981039346656037ull;
#else
    const size_t FNV_prime = 16777619;
    const size_t offset_basis = 2166136261;
#endif
    size_t hash = offset_basis;
    for (const char* lpsz = GetData(); *lpsz != 0; ++lpsz)
    {
        char ch = *lpsz;
#if defined(MIKTEX_WINDOWS)
        if (ch == PathNameUtil::DirectoryDelimiter)
        {
            ch = PathNameUtil::AltDirectoryDelimiter;
        }
        else if (static_cast<unsigned>(ch) >= 128)
        {
            // ignore UTF-8 chars
            continue;
        }
        else if (ch >= 'A' && ch <= 'Z')
        {
            ch = ch - 'A' + 'a';
        }
#endif
        hash ^= (size_t)ch;
        hash *= FNV_prime;
    }
    return hash;
}

string PathName::ToDisplayString(DisplayPathNameOptions options) const
{
#if defined(MIKTEX_WINDOWS)
    return ToDos().ToString();
#else
    return ToString();
#endif
}

PathName& PathName::SetToHomeDirectory()
{
    *this = Helpers::GetHomeDirectory();
    return *this;
}

PathName& PathName::SetToLockDirectory()
{
    *this = Helpers::GetHomeDirectory();
    return *this;
}

PathName& PathName::SetToTempFile()
{
    PathName tmpDir;
    tmpDir.SetToTempDirectory();
    return SetToTempFile(tmpDir);
}
