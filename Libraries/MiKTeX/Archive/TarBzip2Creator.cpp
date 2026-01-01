/**
 * @file TarBzip2Creator.cpp
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

#include "config.h"

#include <memory>

#include <miktex/Core/BZip2Stream>

#include "internal.h"

#include "TarBzip2Creator.h"
#include "TarCreator.h"

using namespace std;

using namespace MiKTeX::Archive;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

void TarBzip2Creator::Create(const PathName& path, const vector<FileSet>& fileSets)
{
    unique_ptr<BZip2Stream> bz2Stream = BZip2Stream::Create(path, false);
    TarCreator::Create(bz2Stream.get(), fileSets);
}
