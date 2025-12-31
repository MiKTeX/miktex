/**
 * @file miktex/Archive/Extractor.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief Extractor interface for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2025 Christian Schenk
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

#include <cstddef>

#include <memory>
#include <string>

#include <miktex/Util/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/Core/Stream>

 /// @namespace MiKTeX::Archive
 /// @brief Extracting files from compressed archives.
MIKTEX_ARCHIVE_BEGIN_NAMESPACE;

enum class ArchiveFileType
{
    None, MSCab, TarBzip2, Zip, Tar, TarLzma, TarXz
};

class MIKTEXNOVTABLE IExtractCallback
{

public:

    virtual void MIKTEXTHISCALL OnBeginFileExtraction(const std::string& fileName, std::size_t uncompressedSize) = 0;
    virtual void MIKTEXTHISCALL OnEndFileExtraction(const std::string& fileName, std::size_t uncompressedSize) = 0;
    virtual bool MIKTEXTHISCALL OnError(const std::string& message) = 0;
};

class MIKTEXNOVTABLE Extractor
{

public:

    virtual MIKTEXTHISCALL ~Extractor() noexcept = 0;
    virtual void MIKTEXTHISCALL Extract(const MiKTeX::Util::PathName& path, const MiKTeX::Util::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& prefix) = 0;

    virtual void MIKTEXTHISCALL Extract(MiKTeX::Core::Stream* stream, const MiKTeX::Util::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& prefix) = 0;

    void MIKTEXTHISCALL Extract(const MiKTeX::Util::PathName& path, const MiKTeX::Util::PathName& destDir, bool makeDirectories)
    {
        return Extract(path, destDir, makeDirectories, nullptr, "");
    }

    void MIKTEXTHISCALL Extract(const MiKTeX::Util::PathName& path, const MiKTeX::Util::PathName& destDir)
    {
        return Extract(path, destDir, false, nullptr, "");
    }

    void MIKTEXTHISCALL Extract(MiKTeX::Core::Stream* stream, const MiKTeX::Util::PathName& destDir, bool makeDirectories)
    {
        return Extract(stream, destDir, makeDirectories, nullptr, "");
    }

    void MIKTEXTHISCALL Extract(MiKTeX::Core::Stream* stream, const MiKTeX::Util::PathName& destDir)
    {
        return Extract(stream, destDir, false, nullptr, "");
    }

    static MIKTEXARCHIVECEEAPI(std::unique_ptr<Extractor>) CreateExtractor(ArchiveFileType archiveFileType);

    static const std::string GetFileNameExtension(ArchiveFileType archiveFileType)
    {
        switch (archiveFileType)
        {
        case ArchiveFileType::MSCab:
            return MIKTEX_CABINET_FILE_SUFFIX;
        case ArchiveFileType::TarBzip2:
            return MIKTEX_TARBZIP2_FILE_SUFFIX;
        case ArchiveFileType::TarLzma:
            return MIKTEX_TARLZMA_FILE_SUFFIX;
        case ArchiveFileType::TarXz:
            return MIKTEX_TARXZ_FILE_SUFFIX;
        case ArchiveFileType::Zip:
            return MIKTEX_ZIP_FILE_SUFFIX;
        case ArchiveFileType::Tar:
            return MIKTEX_TAR_FILE_SUFFIX;
        default:
            MIKTEX_UNEXPECTED();
        }
    }
};

MIKTEX_ARCHIVE_END_NAMESPACE;
