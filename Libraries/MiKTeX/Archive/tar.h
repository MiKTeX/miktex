/**
 * @file tar.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief Internal definitions for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <cstddef>
#include <ctime>

#include <iomanip>
#include <string>

#include <miktex/Core/Debug>
#include <miktex/Util/PathName>

#include "internal.h"

BEGIN_INTERNAL_NAMESPACE;

const std::size_t BLOCKSIZE = 512;

struct Header
{

public:

    Header()
    {
        MIKTEX_ASSERT(sizeof(*this) == BLOCKSIZE);
        memset(this, 0, sizeof(*this));
        SetString(magic, "ustar");
        SetString(version, "00");
        SetString(uname, "root");
        SetOctal(uid, 0);
        SetString(gname, "root");
        SetOctal(gid, 0);
        SetOctal(mode, 0700);
    }

    bool Check() const
    {
        return ComputeChecksum() == GetOctal(checksum);
    }

    void SetChecksum()
    {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(sizeof(checksum) - 2) << std::oct << ComputeChecksum();
        auto s = oss.str();
        if (s.length() != sizeof(checksum) - 2)
        {
            MIKTEX_UNEXPECTED();
        }
        memcpy(checksum, s.c_str(), sizeof(checksum) - 2);
        checksum[sizeof(checksum) - 2] = '\0';
        checksum[sizeof(checksum) - 1] = ' ';
    }

    bool IsZero() const
    {
        for (std::size_t i = 0; i < sizeof(*this); ++i)
        {
            if (reinterpret_cast<const unsigned char*>(this)[i] != 0)
            {
                return false;
            }
        }
        return true;
    }

    std::string GetFileName() const
    {
        if (prefix[0] == 0)
        {
            return GetString(name);
        }
        return GetString(prefix) + "/" + GetString(name);
    }

    void SetFileName(const std::string& fileName)
    {
        if (fileName.length() < sizeof(name))
        {
            SetString(name, fileName);
            memset(prefix, 0, sizeof(prefix));
            return;
        }
        auto pos = fileName.rfind('/');
        if (pos == std::string::npos)
        {
            MIKTEX_UNEXPECTED();
        }
        SetString(prefix, fileName.substr(0, pos));
        SetString(name, fileName.substr(pos + 1));
    }

    std::size_t GetFileSize() const
    {
        return GetOctal(size);
    }

    void SetFileSize(std::size_t size)
    {
        SetOctal(this->size, static_cast<int>(size));
    }

    std::time_t GetLastModificationTime() const
    {
        return GetOctal(mtime);
    }

    void SetLastModificationTime(std::time_t time)
    {
        SetOctal(mtime, static_cast<int>(time));
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

    void SetType(Type type)
    {
        typeflag[0] = static_cast<char>(type);
    }

    bool IsNormalFile() const
    {
        return GetType() == RegularFile || GetType() == AlternateRegularFile;
    }

private:

    template<std::size_t N> std::string GetString(const char(&field)[N]) const
    {
        return std::string(field, strnlen(field, N));
    }

    template<std::size_t N> void SetString(char(&field)[N], const std::string& str)
    {
        if (str.length() > N)
        {
            MIKTEX_UNEXPECTED();
        }
        memset(field, 0, N);
        memcpy(field, str.c_str(), str.length());
    }

    template<std::size_t N> int GetOctal(const char(&field)[N]) const
    {
        return std::stoi(GetString(field), nullptr, 8);
    }

    template<std::size_t N> void SetOctal(char(&field)[N], int value)
    {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(N - 1) << std::oct << value;
        SetString(field, oss.str());
    }

    unsigned ComputeChecksum() const
    {
        unsigned myHeaderCheckSum = 0;
        for (std::size_t i = 0; i < BLOCKSIZE; ++i)
        {
            auto c = reinterpret_cast<const unsigned char*>(this)[i];
            if (offsetof(Header, checksum) <= i && i < offsetof(Header, checksum) + sizeof(checksum))
            {
                c = ' ';
            }
            myHeaderCheckSum += c;
        }
        return myHeaderCheckSum;
    }

    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

END_INTERNAL_NAMESPACE;
