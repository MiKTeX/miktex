/**
 * @file mfapp.cpp
 * @author Christian Schenk
 * @brief MiKTeX METAFONT base implementation
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#include <fmt/format.h>
#include <fmt/ostream.h>

#if defined(MIKTEX_TEXMF_SHARED)
#   define C4PEXPORT MIKTEXDLLEXPORT
#else
#   define C4PEXPORT
#endif
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P.h"

#if defined(MIKTEX_TEXMF_SHARED)
#   define MIKTEXMFEXPORT MIKTEXDLLEXPORT
#else
#   define MIKTEXMFEXPORT
#endif
#define B8C7815676699B4EA2DE96F0BD727276
#include "miktex/TeXAndFriends/MetafontApp.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::TeXAndFriends;

class MetafontApp::impl
{
    public:
    int optBase;
};

MetafontApp::MetafontApp() :
    pimpl(make_unique<impl>())
{
}

MetafontApp::~MetafontApp() noexcept
{
}

void MetafontApp::Init(vector<char*>& args)
{
    TeXMFApp::Init(args);
    IAm(METAFONTEngine);
}

void MetafontApp::Finalize()
{
    TeXMFApp::Finalize();
}

enum
{
    OPT_BISTACK_SIZE,
    OPT_LIG_TABLE_SIZE,
    OPT_PATH_SIZE,
};

void MetafontApp::AddOptions()
{
    TeXMFApp::AddOptions();
    pimpl->optBase = (int)GetOptions().size();
    AddOption("bistack-size", fmt::format(T_("Set {0} to N."), "bistack_size"), FIRST_OPTION_VAL + pimpl->optBase + OPT_BISTACK_SIZE, POPT_ARG_STRING, "N");
    AddOption("lig-table-size", fmt::format(T_("Set {0} to N."), "lig_table_size"), FIRST_OPTION_VAL + pimpl->optBase + OPT_LIG_TABLE_SIZE, POPT_ARG_STRING, "N");
    AddOption("path-size", fmt::format(T_("Set {0} to N."), "path_size"), FIRST_OPTION_VAL + pimpl->optBase + OPT_PATH_SIZE, POPT_ARG_STRING, "N");
}

bool MetafontApp::ProcessOption(int opt, const string& optArg)
{
    bool done = true;
    switch (opt - FIRST_OPTION_VAL - pimpl->optBase)
    {
    case OPT_BISTACK_SIZE:
        GetUserParams()["bistack_size"] = std::stoi(optArg);
        break;
    case OPT_LIG_TABLE_SIZE:
        GetUserParams()["lig_table_size"] = std::stoi(optArg);
        break;
    case OPT_PATH_SIZE:
        GetUserParams()["path_size"] = std::stoi(optArg);
        break;
    default:
        done = TeXMFApp::ProcessOption(opt, optArg);
        break;
    }
    return done;
}
