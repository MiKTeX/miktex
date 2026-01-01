/**
 * @file TarBzip2Creator.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief TarBzip2Creator implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

 #pragma once

#include "TarCreator.h"

BEGIN_INTERNAL_NAMESPACE;

class TarBzip2Creator : public TarCreator
{
public:
    void MIKTEXTHISCALL Create(const MiKTeX::Util::PathName& path, const std::vector<MiKTeX::Archive::FileSet>& fileSets) override;
};

END_INTERNAL_NAMESPACE;
