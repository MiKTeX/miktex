/**
 * @file TarLzmaExtractor.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief TarLzmaExtractor implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2025 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include "TarExtractor.h"

BEGIN_INTERNAL_NAMESPACE;

class TarLzmaExtractor : public TarExtractor
{
public:
    void MIKTEXTHISCALL Extract(const MiKTeX::Util::PathName& path, const MiKTeX::Util::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& prefix) override;
};

END_INTERNAL_NAMESPACE;
