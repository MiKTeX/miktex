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
            return "set-option --name=NAME [--value=VALUE]";
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
    OPT_NAME,
    OPT_VALUE,
};

static const struct poptOption options[] =
{
    {
        "name", 0,
        POPT_ARG_STRING, nullptr,
        OPT_NAME,
        T_("Specify the option name."),
        "NAME"
    },
    {
        "value", 0,
        POPT_ARG_STRING, nullptr,
        OPT_VALUE,
        T_("Specify the option value."),
        "NAME"
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int SetOptionCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string name;
    string value;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_NAME:
            name = popt.GetOptArg();
            break;
        case OPT_VALUE:
            value = popt.GetOptArg();
            break;
        }
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    if (!popt.GetLeftovers().empty())
    {
        ctx.ui->IncorrectUsage(T_("unexpected command arguments"));
    }
    if (name.empty())
    {
        ctx.ui->IncorrectUsage(T_("expected --name=NAME"));
    }
    FontMapManager mgr;
    mgr.Init(ctx);
    mgr.SetOption(name, value);
    return 0;
}
