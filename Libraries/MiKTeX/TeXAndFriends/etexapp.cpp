/**
 * @file etexapp.cpp
 * @author Christian Schenk
 * @brief MiKTeX e-TeX base implementation
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

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
#include "miktex/TeXAndFriends/ETeXApp.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::TeXAndFriends;

class ETeXApp::impl
{
public:
    bool enableETeX;
    int optBase;
};

ETeXApp::ETeXApp() :
    pimpl(make_unique<impl>())
{
}

ETeXApp::~ETeXApp() noexcept
{
}

void ETeXApp::Init(vector<char*>& args)
{
    TeXApp::Init(args);
    pimpl->enableETeX = false;
}

void ETeXApp::OnTeXMFStartJob()
{
    TeXApp::OnTeXMFStartJob();
}

void ETeXApp::Finalize()
{
    TeXApp::Finalize();
}

enum
{
    OPT_ENABLE_ETEX,
};

void ETeXApp::AddOptions()
{
    TeXApp::AddOptions();
    pimpl->optBase = (int)GetOptions().size();
    AddOption("enable-etex", T_("Enable e-TeX extensions."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ENABLE_ETEX);
    // supported Web2C options
    AddOption("etex", "enable-etex");
}

bool ETeXApp::ProcessOption(int optchar, const string& optArg)
{
    bool done = true;
    switch (optchar - FIRST_OPTION_VAL - pimpl->optBase)
    {
    case OPT_ENABLE_ETEX:
        pimpl->enableETeX = true;
        break;
    default:
        done = TeXApp::ProcessOption(optchar, optArg);
        break;
    }
    return done;
}

bool ETeXApp::ETeXP() const
{
    return pimpl->enableETeX;
}
