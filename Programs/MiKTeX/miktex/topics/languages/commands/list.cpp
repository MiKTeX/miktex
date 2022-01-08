/**
 * @file topics/languanges/commands/list.cpp
 * @author Christian Schenk
 * @brief languages list
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

#include <miktex/Core/Session>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

namespace
{
    class ListCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("List LaTeX language definitions");
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

        const std::string defaultTemplate = "{key} {loader}";
    };
}

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Languages;

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
    for (const LanguageInfo& l : ctx.session->GetLanguages())
    {
        ctx.ui->Output(fmt::format(outputTemplate,
            fmt::arg("custom", l.custom),
            fmt::arg("exclude", l.exclude),
            fmt::arg("hyphenation", l.hyphenation),
            fmt::arg("key", l.key),
            fmt::arg("lefthyphenmin", l.lefthyphenmin),
            fmt::arg("righthyphenmin", l.righthyphenmin),
            fmt::arg("loader", l.loader),
            fmt::arg("luaspecial", l.luaspecial),
            fmt::arg("patterns", l.patterns),
            fmt::arg("righthyphenmin", l.righthyphenmin),
            fmt::arg("synonyms", l.synonyms)
        ));
    }
    return 0;
}
