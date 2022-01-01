/**
 * @file topics/formats/commands/build.cpp
 * @author Christian Schenk
 * @brief formats build
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

#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

#include "FormatsManager.h"

namespace
{
    class BuildCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Build TeX format files");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "build";
        }

        std::string Synopsis() override
        {
            return "build [--engine=ENGINE] [--name=NAME]";
        }
    };
}

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Formats;

unique_ptr<Command> Commands::Build()
{
    return make_unique<BuildCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_ENGINE,
    OPT_NAME,
};

static const struct poptOption options[] =
{
    {
        "engine", 0,
        POPT_ARG_STRING, nullptr,
        OPT_ENGINE,
        T_("Engine to be used."),
        T_("ENGINE")
    },
    {
        "name", 0,
        POPT_ARG_STRING, nullptr,
        OPT_NAME,
        T_("Specify the format name."),
        "DIR"
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int BuildCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string engine;
    string name;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_ENGINE:
            engine = popt.GetOptArg();
            break;
        case OPT_NAME:
            name = popt.GetOptArg();
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
    FormatsManager mgr;
    mgr.Init(ctx);
    if (name.empty())
    {
        for (auto& f : mgr.Formats())
        {
            if (!engine.empty() && engine != f.compiler)
            {
                continue;
            }
            mgr.Build(f.key);
        }
    }
    else
    {
        if (!engine.empty())
        {
            auto formatInfo = mgr.Format(name);
            if (engine != formatInfo.compiler)
            {
                ctx.ui->FatalError(fmt::format(T_("{0}: cannot be built by {1}"), name, engine));
            }
        }
        mgr.Build(name);
    }
    return 0;
}
