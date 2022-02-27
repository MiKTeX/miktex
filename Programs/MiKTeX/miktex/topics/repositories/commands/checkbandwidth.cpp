/**
 * @file topics/repositories/commands/check.cpp
 * @author Christian Schenk
 * @brief repositories check
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
    class CheckCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Check the bandwidth of MiKTeX package repositories");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "check-bandwidth";
        }

        std::string Synopsis() override
        {
            return "check-bandwidth [--template=TEMPLATE] [--url=URL]";
        }

        const std::string defaultTemplate = "{bandwidth:.2f}\t{url}";
    };
}

using namespace std;

using namespace MiKTeX::Packages;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Repositories;

unique_ptr<Command> Commands::Check()
{
    return make_unique<CheckCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_TEMPLATE,
    OPT_URL
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
    {
        "url", 0,
        POPT_ARG_STRING, nullptr,
        OPT_URL,
        T_("Specify the repository URL."),
        "URL"
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int CheckCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string outputTemplate = this->defaultTemplate;
    vector<string> toBeChecked;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_TEMPLATE:
            outputTemplate = Unescape(popt.GetOptArg());
            break;
        case OPT_URL:
            toBeChecked.push_back(popt.GetOptArg());
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
    vector<RepositoryInfo> repositories;
    ctx.packageManager->DownloadRepositoryList();
    if (toBeChecked.empty())
    {
        repositories = ctx.packageManager->GetRepositories();
    }
    else
    {
        for (auto u : toBeChecked)
        {
            repositories.push_back(ctx.packageManager->VerifyPackageRepository(u));
        }
    }
    if (repositories.empty())
    {
        ctx.ui->Verbose(0, T_("No package repositories are currently available."));
    }
    sort(repositories.begin(), repositories.end(), CountryComparer());
    for (RepositoryInfo& ri : repositories)
    {
        if (ctx.program->Canceled())
        {
            break;
        }
        ri = ctx.packageManager->CheckPackageRepository(ri.url);
        ctx.ui->Output(Format(outputTemplate, ri));
    }
    return 0;
}
