/* shims/updmap.cpp:

   Copyright (C) 2021 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "updmap.h"

#define T_(x) MIKTEXTEXT(x)

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;

enum Option
{
    OPT_AAA = 1,
    OPT_NOHASH,
    OPT_NOMKMAP,
    OPT_QUIET,
    OPT_SETOPTION,
    OPT_SHOWOPTION,
    OPT_SYS,
    OPT_USER,
};

static const struct poptOption aoption[] =
{
    { "nohash", 0, POPT_ARG_NONE, nullptr, OPT_NOHASH, nullptr, nullptr },
    { "nomkmap", 0, POPT_ARG_NONE, nullptr, OPT_NOMKMAP, nullptr, nullptr },
    { "quiet", 0, POPT_ARG_NONE, nullptr, OPT_QUIET, nullptr, nullptr },
    { "setoption", 0, POPT_ARG_STRING, nullptr, OPT_SETOPTION, nullptr, nullptr },
    { "showoption", 0, POPT_ARG_STRING, nullptr, OPT_SHOWOPTION, nullptr, nullptr },
    { "sys", 0, POPT_ARG_NONE, nullptr, OPT_SYS, nullptr, nullptr },
    { "user", 0, POPT_ARG_NONE, nullptr, OPT_USER, nullptr, nullptr },
    POPT_AUTOHELP
    POPT_TABLEEND
};

void Shims::updmap(OneMiKTeXUtility::ApplicationContext* ctx, vector<string>& arguments)
{
    vector<const char*> argv;
    argv.reserve(arguments.size() + 1);
    for (int idx = 0; idx < arguments.size(); ++idx)
    {
        argv.push_back(arguments[idx].c_str());
    }
    argv.push_back(nullptr);

    PoptWrapper popt(static_cast<int>(arguments.size()), &argv[0], aoption);

    int option;

    bool optSetOption = false;
    bool optShowOption = false;
    string optionName;

    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_NOHASH:
            // NOOP
            break;
        case OPT_NOMKMAP:
            // NOOP
            break;
        case OPT_QUIET:
            // NOOP
            break;
        case OPT_SETOPTION:
            optSetOption = true;
            optionName = popt.GetOptArg();
            break;
        case OPT_SHOWOPTION:
            optShowOption = true;
            optionName = popt.GetOptArg();
            break;
        case OPT_SYS:
            if (!ctx->session->IsSharedSetup())
            {
                ctx->ui->FatalError(T_("option --sys only makes sense for a shared MiKTeX setup"));
            }
            if (!ctx->session->RunningAsAdministrator())
            {
               ctx->ui->Warning(T_("option --sys may require administrator privileges"));
            }
            ctx->session->SetAdminMode(true, false);
            break;
        case OPT_USER:
            // NOOP
            break;
        }
    }

    if (option != -1)
    {
        string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
        msg += ": ";
        msg += popt.Strerror(option);
        ctx->ui->FatalError(msg);
    }

    auto leftovers = popt.GetLeftovers();

    if (optShowOption)
    {
        arguments = {"fontmaps", "show-option", optionName};
    }
    else if (optSetOption)
    {
        if (leftovers.empty())
        {
            ctx->ui->BadUsage(fmt::format(T_("missing value for option {0}"), optionName), "");
        }
        arguments = {"fontmaps", "set-option", optionName, leftovers[0]};
    }
    else
    {
        arguments = {"fontmaps", "update"};
    }
}
