/**
 * @file topics/formats/commands/list.cpp
 * @author Christian Schenk
 * @brief formats list
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
#include <miktex/Util/StringUtil>

#include "internal.h"

#include "commands.h"

namespace
{
class CountryComparer
{
public:
    inline bool operator() (const MiKTeX::Packages::RepositoryInfo& lhs, const MiKTeX::Packages::RepositoryInfo& rhs)
    {
        if (lhs.ranking == rhs.ranking)
        {
            return MiKTeX::Util::StringCompare(lhs.country.c_str(), rhs.country.c_str(), true) < 0;
        }
        else
        {
            return lhs.ranking < rhs.ranking;
        }
    }
};

    class ListCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("List MiKTeX package repositories");
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

        const std::string defaultTemplate = "{url}";
    };
}

using namespace std;

using namespace MiKTeX::Packages;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Repositories;

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
            outputTemplate = Unescape(popt.GetOptArg());
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
    ctx.packageManager->DownloadRepositoryList();
    vector<RepositoryInfo> repositories = ctx.packageManager->GetRepositories();
    if (repositories.empty())
    {
        ctx.ui->Verbose(0, T_("No package repositories are currently available."));
    }
    sort(repositories.begin(), repositories.end(), CountryComparer());
    for (const RepositoryInfo& ri : repositories)
    {
        ctx.ui->Output(fmt::format(outputTemplate,
            fmt::arg("url", ri.url)
        ));
    }
    return 0;
}
