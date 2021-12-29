/**
 * @file topics/fontmaps/commands/set-option.cpp
 * @author Christian Schenk
 * @brief fontmaps set-option
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "internal.h"

#include "commands.h"

#include "FontMapManager.h"

class SetOptionCommand :
    public OneMiKTeXUtility::Topics::Command
{
    std::string Description() override
    {
        return T_("Set OPTION to VALUE");
    }

    int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

    std::string Name() override
    {
        return "set-option";
    }

    std::string Synopsis() override
    {
        return "set-option OPTION VALUE";
    }
};

using namespace std;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FontMaps;

unique_ptr<Command> Commands::SetOption()
{
    return make_unique<SetOptionCommand>();
}

int SetOptionCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 4)
    {
        ctx.ui->IncorrectUsage(T_("expected two arguments: OPTION VALUE"));
    }
    FontMapManager updmap;
    updmap.Init(ctx);
    updmap.SetOption(arguments[2], arguments[3]);
    return 0;
}
