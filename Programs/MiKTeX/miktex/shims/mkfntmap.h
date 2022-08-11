/**
 * @file shims/mkfntmap.h
 * @author Christian Schenk
 * @brief mkfntmap shim
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>
#include <vector>

#include "internal.h"

namespace OneMiKTeXUtility::Shims
{
    void mkfntmap(OneMiKTeXUtility::ApplicationContext* ctx, std::vector<std::string>& arguments);
}
