/**
 * @file topics/formats/commands/list.cpp
 * @author Christian Schenk
 * @brief formats list
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

#include "FormatsManager.h"

namespace
{
    class ListCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("List TeX formats");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "list";
        }

        std::string Synopsis() override
        {
            return "list [--format=TEMPLATE]";
        }

        const std::string defaultTemplate = "{key}";
    };
}

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Formats;

unique_ptr<Command> Commands::List()
{
    return make_unique<ListCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_FORMAT,
};

static const struct poptOption options[] =
{
    {
        "format", 0,
        POPT_ARG_STRING, nullptr,
        OPT_FORMAT,
        T_("Specify output format template."),
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
    string formatTemplate = this->defaultTemplate;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_FORMAT:
            formatTemplate = popt.GetOptArg();
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
    for (auto& f : mgr.Formats())
    {
        ctx.ui->Output(fmt::format(formatTemplate,
            fmt::arg("arguments", f.arguments),
            fmt::arg("compiler", f.compiler),
            fmt::arg("custom", f.custom),
            fmt::arg("description", f.description),
            fmt::arg("exclude", f.exclude),
            fmt::arg("inputFile", f.inputFile),
            fmt::arg("key", f.key),
            fmt::arg("name", f.name),
            fmt::arg("noExecutable", f.noExecutable),
            fmt::arg("outputFile", f.outputFile),
            fmt::arg("preloaded", f.preloaded)
        ));
    }
    return 0;
}
