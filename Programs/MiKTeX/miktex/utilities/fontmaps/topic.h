/**
 * @file utilities/fontmaps/topic.h
 * @author Christian Schenk
 * @brief fontmaps topic
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

namespace OneMiKTeXUtility::Topics::FontMaps
{
    std::unique_ptr<Topics::Topic> Create();
}
