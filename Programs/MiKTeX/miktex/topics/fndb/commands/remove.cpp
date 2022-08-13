/**
 * @file topics/fndb/commands/remove.cpp
 * @author Christian Schenk
 * @brief fndb remove
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

#include <miktex/Configuration/ConfigurationProvider>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Util/PathName>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

namespace
{
    class RemoveCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Remove the file name database");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "remove";
        }

        std::string Synopsis() override
        {
            return "remove";
        }
    };
}

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FNDB;

unique_ptr<Command> Commands::Remove()
{
    return make_unique<RemoveCommand>();
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

int RemoveCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
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
    if (!popt.GetLeftovers().empty())
    {
        ctx.ui->IncorrectUsage(T_("unexpected command arguments"));
    }
    ctx.session->UnloadFilenameDatabase();
    size_t nRoots = ctx.session->GetNumberOfTEXMFRoots();
    for (unsigned r = 0; r < nRoots; ++r)
    {
        PathName path = ctx.session->GetFilenameDatabasePathName(r);
        ctx.ui->Verbose(1, fmt::format(T_("Removing {0}..."), Q_(path.ToDisplayString())));
        File::Delete(path, { FileDeleteOption::TryHard });
        PathName changeFile = path;
        changeFile.SetExtension(MIKTEX_FNDB_CHANGE_FILE_SUFFIX);
        if (File::Exists(changeFile))
        {
            ctx.ui->Verbose(1, fmt::format(T_("Removing {0}..."), Q_(changeFile.ToDisplayString())));
            File::Delete(changeFile, { FileDeleteOption::TryHard });
        }
    }
    return 0;
}
