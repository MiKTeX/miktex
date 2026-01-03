/**
 * @file miktex/Archive/Creator.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief Creator interface for MiKTeX Archive
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <miktex/First>
#include <miktex/Definitions>

#include "config.h"

#include <memory>

#include <miktex/Core/Stream>
#include <miktex/Util/PathName>

#include "common.h"

MIKTEX_ARCHIVE_BEGIN_NAMESPACE;

struct FileSet {
    MiKTeX::Util::PathName baseDir;
    std::string prefix;
    std::vector<std::string> names;
};

class MIKTEXNOVTABLE Creator
{

public:

    virtual MIKTEXTHISCALL ~Creator() noexcept = 0;

    virtual void MIKTEXTHISCALL Create(const MiKTeX::Util::PathName& path, const std::vector<FileSet>& fileSets) = 0;
    virtual void MIKTEXTHISCALL Create(MiKTeX::Core::Stream* stream, const std::vector<FileSet>& fileSets) = 0;

    static MIKTEXARCHIVECEEAPI(std::unique_ptr<Creator>) New(ArchiveFileType archiveFileType);

};

MIKTEX_ARCHIVE_END_NAMESPACE;
