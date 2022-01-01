/**
 * @file utilities/filesystem/topic.h
 * @author Christian Schenk
 * @brief filesystem topic
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <memory>

#include "internal.h"

#include "utilities/Topic.h"

namespace OneMiKTeXUtility::Topics::FileSystem
{
    std::unique_ptr<OneMiKTeXUtility::Topics::Topic> Create();
}
