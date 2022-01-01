/**
 * @file utilities/filetypes/topic.h
 * @author Christian Schenk
 * @brief filetypes utility
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

namespace OneMiKTeXUtility::Utilities::FileTypes
{
    std::unique_ptr<Utilities::Utility> Create();
}
