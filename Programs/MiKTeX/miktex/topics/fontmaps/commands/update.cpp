/**
 * @file update.cpp
 * @author Christian Schenk
 * @brief Update TeX font map files
 *
 * @copyright Copyright © 2002-2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 *
 * The code in this file is based on the updmap Perl script
 * (updmap.pl):
 *
 * @code {.unparsed}
 * # Copyright 2011-2021 Norbert Preining
 * # This file is licensed under the GNU General Public License version 2
 * # or any later version.
 * @endcode
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

class UpdateCommand :
    public OneMiKTeXUtility::Topics::Command
{
    std::string Description() override
    {
        return T_("Update TeX font map files");
    }

    int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

    std::string Name() override
    {
        return "update";
    }

    std::string Synopsis() override
    {
        return "update [--force] [--output-directory=DIR]";
    }
};

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FontMaps;

unique_ptr<Command> Commands::Update()
{
    return make_unique<UpdateCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_FORCE,
    OPT_OUTPUT_DIRECTORY,
};

static const struct poptOption update_options[] =
{
    {
        "force", 0,
        POPT_ARG_NONE, nullptr,
        OPT_FORCE,
        T_("Force re-generation of apparently up-to-date fontconfig cache files, overriding the timestamp checking."),
        nullptr,
    },
    {
        "output-directory", 0,
        POPT_ARG_STRING, nullptr,
        OPT_OUTPUT_DIRECTORY,
        T_("Set the output directory."),
        "DIR"
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int UpdateCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    vector<const char*> argv;
    argv.reserve(arguments.size() + 1);
    for (int idx = 0; idx < arguments.size(); ++idx)
    {
        argv.push_back(arguments[idx].c_str());
    }
    argv.push_back(nullptr);

    PoptWrapper popt(static_cast<int>(arguments.size()), &argv[0], update_options);

    int option;

    bool force = false;
    string outputDirectory;

    vector<string> newargs;

    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_FORCE:
            force = true;
            break;
        case OPT_OUTPUT_DIRECTORY:
            outputDirectory = popt.GetOptArg();
            break;
        }
    }

    if (option != -1)
    {
        string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
        msg += ": ";
        msg += popt.Strerror(option);
        ctx.ui->IncorrectUsage(msg);
    }

    if (!popt.GetLeftovers().empty())
    {
        ctx.ui->IncorrectUsage(T_("unexpected command arguments"));
    }

    FontMapManager updmap;
    updmap.Init(ctx);
    updmap.WriteMapFiles(force, outputDirectory);

    return 0;
}
