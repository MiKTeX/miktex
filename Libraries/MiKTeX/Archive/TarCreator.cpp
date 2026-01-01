/**
 * @file TarCreator.cpp
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

#include "config.h"

#include "internal.h"

#include "TarCreator.h"

using namespace std;

using namespace MiKTeX::Archive;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

TarCreator::TarCreator()
{
}

TarCreator::~TarCreator()
{
}

void TarCreator::Create(const PathName& path, const vector<FileSet>& fileSets)
{
}

void TarCreator::Create(Stream* stream, const vector<FileSet>& fileSets)
{
}
