/**
 * @file topics/repositories/commands/info.cpp
 * @author Christian Schenk
 * @brief repositories info
 *
 * @copyright Copyright © 2022 Christian Schenk
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

#include <miktex/PackageManager/PackageManager>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

#include "private.h"

namespace
{
    class InfoCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Get information about MiKTeX package repositories");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "info";
        }

        std::string Synopsis() override
        {
            return "info [--template <template>] <url-or-directory>";
        }

        const std::string defaultTemplate = R"xYz(url: {url}
country: {country}
version: {version})xYz";
    };
}

using namespace std;

using namespace MiKTeX::Packages;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Repositories;

unique_ptr<Command> Commands::Info()
{
    return make_unique<InfoCommand>();
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

int InfoCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
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
            outputTemplate = Unescape(popt.GetOptArg());
            break;
        }
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    auto leftovers = popt.GetLeftovers();
    if (leftovers.size() != 1)
    {
        ctx.ui->IncorrectUsage(T_("expected one <url-or-directory> argument"));
    }
    RepositoryInfo repositoryInfo;
    if (ctx.packageManager->TryGetRepositoryInfo(leftovers[0], repositoryInfo))
    {
        ctx.ui->Output(Format(outputTemplate, repositoryInfo));
    }    
    return 0;
}
