/**
 * @file topics/packages/commands/list.cpp
 * @author Christian Schenk
 * @brief packages list
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
#include <set>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Util/PathName>
#include <miktex/Util/PathNameUtil>
#include <miktex/Util/StringUtil>
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
            return T_("List MiKTeX packages");
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

        const std::string defaultTemplate = "{id}";
    };
}

class PackageInfoComparer
{
public:
    bool operator() (const MiKTeX::Packages::PackageInfo& pi1, const MiKTeX::Packages::PackageInfo& pi2) const
    {
        return MiKTeX::Util::PathName::Compare(pi1.id, pi2.id) < 0;
    }
};

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Packages;

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
    auto packageManager = PackageManager::Create();
    auto packageIterator = packageManager->CreateIterator();
    PackageInfo packageInfo;
    set<PackageInfo, PackageInfoComparer> setPi;
    while (packageIterator->GetNext(packageInfo))
    {
        if (packageInfo.IsPureContainer())
        {
            continue;
        }
        setPi.insert(packageInfo);
    }
    if (setPi.empty())
    {
        ctx.ui->FatalError(T_("The package database files have not been installed."));
    }
    auto delim = PathNameUtil::PathNameDelimiter;
    for (set<PackageInfo, PackageInfoComparer>::const_iterator it = setPi.begin(); it != setPi.end(); ++it)
    {
        ctx.ui->Output(fmt::format(outputTemplate,
            fmt::arg("archiveFileSize", it->archiveFileSize), // FIXME
            fmt::arg("copyrightOwner", it->copyrightOwner),
            fmt::arg("copyrightYear", it->copyrightYear),
            fmt::arg("creator", it->creator),
            fmt::arg("ctanPath", it->ctanPath),
            fmt::arg("description", it->description),
            fmt::arg("digest", it->digest),
            fmt::arg("displayName", it->displayName),
            fmt::arg("docFiles", StringUtil::Flatten(it->docFiles, delim)),
            fmt::arg("id", it->id),
            fmt::arg("isContained", it->IsContained()),
            fmt::arg("isContainer", it->IsContainer()),
            fmt::arg("isInstalled", it->IsInstalled()),
            fmt::arg("isInstalledCommon", it->IsInstalled(ConfigurationScope::Common)),
            fmt::arg("isInstalledUser", it->IsInstalled(ConfigurationScope::User)),
            fmt::arg("isObsolete", it->isObsolete),
            fmt::arg("isPureContainer", it->IsPureContainer()),
            fmt::arg("isRemovable", it->isRemovable),
            fmt::arg("licenseType", it->licenseType),
            fmt::arg("numFiles", it->GetNumFiles()),
            fmt::arg("refCount", it->GetRefCount()),
            fmt::arg("releaseState", it->releaseState),
            fmt::arg("requiredBy", StringUtil::Flatten(it->requiredBy, delim)),
            fmt::arg("requiredPackages", StringUtil::Flatten(it->requiredPackages, delim)),
            fmt::arg("runFiles", StringUtil::Flatten(it->runFiles, delim)),
            fmt::arg("size", it->GetSize()),
            fmt::arg("sizeDocFiles", it->sizeDocFiles),
            fmt::arg("sizeRunFiles", it->sizeRunFiles),
            fmt::arg("sizeSourceFiles", it->sizeSourceFiles),
            fmt::arg("sourceFiles", StringUtil::Flatten(it->sourceFiles, delim)),
            fmt::arg("targetSystem", it->targetSystem),
            fmt::arg("timeInstalled", it->GetTimeInstalled()),
            fmt::arg("timeInstalledCommon", it->timeInstalledCommon),
            fmt::arg("timeInstalledUser", it->timeInstalledUser),
            fmt::arg("timePackaged", it->timePackaged),
            fmt::arg("title", it->title),
            fmt::arg("version", it->version),
            fmt::arg("versionDate", it->versionDate)
        ));
    }
    return 0;
}
