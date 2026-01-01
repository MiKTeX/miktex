/**
 * @file TarCreator.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief TarCreator implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <miktex/Trace/TraceStream>

#include "miktex/Archive/Creator"

#include "internal.h"

BEGIN_INTERNAL_NAMESPACE;

class TarCreator : public MiKTeX::Archive::Creator
{

public:

    TarCreator();
    MIKTEXTHISCALL ~TarCreator() override;

    void MIKTEXTHISCALL Create(const MiKTeX::Util::PathName& path, const std::vector<MiKTeX::Archive::FileSet>& fileSets) override;
    void MIKTEXTHISCALL Create(MiKTeX::Core::Stream* stream, const std::vector<MiKTeX::Archive::FileSet>& fileSets) override;
};

END_INTERNAL_NAMESPACE;
