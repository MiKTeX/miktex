/**
 * @file shims/mkfntmap.cpp
 * @author Christian Schenk
 * @brief mkfntmap shim
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>
#include <vector>

#include "internal.h"

#include "mkfntmap.h"

using namespace std;

using namespace OneMiKTeXUtility;

void Shims::mkfntmap(vector<string>& arguments)
{
    arguments.insert(arguments.begin(), { "fontmaps", "refresh" });
}
