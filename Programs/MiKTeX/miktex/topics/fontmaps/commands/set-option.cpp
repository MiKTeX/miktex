/**
 * @file set-option.cpp
 * @author Christian Schenk
 * @brief Update TeX font map files
 *
 * @copyright Copyright © 2002-2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 *
 * The code in this file is based on the updmap Perl script
 * (updmap.pl):
 *
 * @code {.unparsed}
 * # Copyright 2011-2021 Norbert Preining
 * # This file is licensed under the GNU General Public License version 2
 * # or any later version.
 * @endcode
 */

#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "internal.h"

#include "commands.h"

#include "FontMapManager.h"

using namespace std;

using namespace OneMiKTeXUtility;

int Topics::FontMaps::Commands::SetOption(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 4)
    {
        ctx.ui->BadUsage(T_("expected two arguments: OPTION VALUE"), "");
    }
    FontMapManager updmap;
    updmap.Init(ctx);
    updmap.SetOption(arguments[2], arguments[3]);
    return 0;
}
