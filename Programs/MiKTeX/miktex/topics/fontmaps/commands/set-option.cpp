/**
 * @file topics/fontmaps/commands/set-option.cpp
 * @author Christian Schenk
 * @brief fontmaps set-option
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <config.h>

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

#include "FontMapManager.h"

namespace
{
    class SetOptionCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Set a configuration option");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "set-option";
        }

        std::string Synopsis() override
        {
            return "set-option <name> [<value>]";
        }
    };
}

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FontMaps;

unique_ptr<Command> Commands::SetOption()
{
    return make_unique<SetOptionCommand>();
}

enum Option
{
    OPT_AAA = 1,
};

static const struct poptOption options[] =
{
    POPT_AUTOHELP
    POPT_TABLEEND
};

int SetOptionCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    while ((option = popt.GetNextOpt()) >= 0)
    {
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    auto leftOvers = popt.GetLeftovers();
    if (leftOvers.empty())
    {
        ctx.ui->IncorrectUsage(T_("expected <name> argument"));
    }
    string name = leftOvers[0];
    string value;
    if (leftOvers.size() == 2)
    {
        value = leftOvers[1];
    }
    else if (leftOvers.size() > 2)
    {
        ctx.ui->IncorrectUsage(T_("too many arguments"));
    }
    FontMapManager mgr;
    mgr.Init(ctx);
    mgr.SetOption(name, value);
    return 0;
}
