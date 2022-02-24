/**
 * @file topics/packages/topic.h
 * @author Christian Schenk
 * @brief formats topic
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <memory>

#include "internal.h"

#include "topics/Topic.h"

namespace OneMiKTeXUtility::Topics::Packages
{
    std::unique_ptr<Topics::Topic> Create();
}
