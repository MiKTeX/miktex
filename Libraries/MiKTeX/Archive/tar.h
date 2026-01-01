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
    }

    bool Check() const
    {
        unsigned myHeaderCheckSum = 0;
        std::size_t i;
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
        for (std::size_t i = 0; i < sizeof(*this); ++i)
        {
            if (reinterpret_cast<const unsigned char*>(this)[i] != 0)
            {
                return false;
            }
        }
        return true;
    }

    MiKTeX::Util::PathName GetFileName() const
    {
        MiKTeX::Util::PathName ret;
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

    std::size_t GetFileSize() const
    {
        return GetOctal(size);
    }

    std::time_t GetLastModificationTime() const
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

    std::string GetLinkName() const
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

    std::string GetOwner() const
    {
        return GetString(uname);
    }

    std::string GetGroup() const
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

    template<std::size_t N> std::string GetString(const char(&field)[N]) const
    {
        char fieldz[N + 1];
        memcpy(fieldz, field, N);
        fieldz[N] = 0;
        return fieldz;
    }

    template<std::size_t N> int GetOctal(const char(&field)[N]) const
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

END_INTERNAL_NAMESPACE;
