/**
 * @file utilities/filesystem/topic.h
 * @author Christian Schenk
 * @brief filesystem utility
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

#include "utilities/Utility.h"

namespace OneMiKTeXUtility::Utilities::FileSystem
{
    std::unique_ptr<OneMiKTeXUtility::Utilities::Utility> Create();
}
