/**
 * @file FileSystemWatcher.cpp
 * @author Christian Schenk
 * @brief File system watcher
 *
 * @copyright Copyright © 2021-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <miktex/Core/FileSystemWatcher>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

FileSystemWatcher::~FileSystemWatcher() noexcept
{
}
