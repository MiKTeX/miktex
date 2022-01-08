/**
 * @file topics/links/commands/list.cpp
 * @author Christian Schenk
 * @brief links list
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
#include <miktex/Util/StringUtil>

#include "internal.h"

#include "commands.h"

#include "LinksManager.h"

namespace
{
    class ListCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("List links");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "list";
        }

        std::string Synopsis() override
        {
            return "list [--template=TEMPLATE]";
        }

        const std::string defaultTemplate = "{linkType} {linkName} -> {target}";
    };
}

using namespace std;

using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Links;

unique_ptr<Command> Commands::List()
{
    return make_unique<ListCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_TEMPLATE,
};

static const struct poptOption options[] =
{
    {
        "template", 0,
        POPT_ARG_STRING, nullptr,
        OPT_TEMPLATE,
        T_("Specify the output template."),
        "TEMPLATE"
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int ListCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string outputTemplate = this->defaultTemplate;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_TEMPLATE:
            outputTemplate = popt.GetOptArg();
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
    LinksManager mgr;
    mgr.Init(ctx);
    for (const auto& f : mgr.Links())
    {
        for (const auto& n : f.linkNames)
        {
            ctx.ui->Output(fmt::format(outputTemplate,
                fmt::arg("linkName", PathName(n).ToDisplayString()),
                fmt::arg("linkType", f.linkType),
                fmt::arg("target", PathName(f.target).ToDisplayString())
            ));
        }
    }
    return 0;
}
