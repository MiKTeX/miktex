/**
 * @file winDirectoryLister.h
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

#pragma once

#include <miktex/Core/DirectoryLister>
#include <miktex/Util/PathName>

CORE_INTERNAL_BEGIN_NAMESPACE;

class winDirectoryLister:
    public MiKTeX::Core::DirectoryLister
{

public:

    void MIKTEXTHISCALL Close() override;
    bool MIKTEXTHISCALL GetNext(MiKTeX::Core::DirectoryEntry& direntry) override;
    bool MIKTEXTHISCALL GetNext(MiKTeX::Core::DirectoryEntry2& direntry2) override;

    winDirectoryLister(const MiKTeX::Util::PathName& directory, const char* lpszPattern, int options);
    virtual MIKTEXTHISCALL ~winDirectoryLister();

private:

    MiKTeX::Util::PathName directory;
    HANDLE handle = INVALID_HANDLE_VALUE;
    std::string pattern;
    int options = (int)Options::None;
    
    friend class MiKTeX::Core::DirectoryLister;
};

CORE_INTERNAL_END_NAMESPACE;
