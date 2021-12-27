/**
 * @file show-option.cpp
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

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "internal.h"

#include "commands.h"

#include "FontMapManager.h"

class ShowOptionCommand :
    public OneMiKTeXUtility::Topics::Command
{
    std::string Description() override
    {
        return T_("Show value of OPTION");
    }

    int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

    std::string Name() override
    {
        return "show-option";
    }

    std::string Synopsis() override
    {
        return "show-option OPTION";
    }
};

using namespace std;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FontMaps;

unique_ptr<Command> Commands::ShowOption()
{
    return make_unique<ShowOptionCommand>();
}

int ShowOptionCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 3)
    {
        ctx.ui->IncorrectUsage(T_("expected one argument: OPTION"));
    }
    FontMapManager updmap;
    updmap.Init(ctx);
    ctx.ui->Output(updmap.Option(arguments[2]));
    return 0;
}
