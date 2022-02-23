/**
 * @file touch.cpp
 * @author Christian Schenk
 * @brief Touch output files
 *
 * @copyright Copyright © 2008-2022 Christian Schenk
 *
 * This file is part of MiKTeX Touch.
 *
 * MiKTeX Touch is licensed under GNU General Public License version 2 or any
 * later version.
 */

#include <cstdio>
#include <cstdlib>

#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Session>
#include <miktex/Core/Utils>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>

#include "touch-version.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <locale>

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

#define T_(x) MIKTEXTEXT(x)

enum Option
{
    OPT_AAA = 1000,
    OPT_TIME_STAMP,
    OPT_VERSION,
};

const struct poptOption aoption[] =
{
    {
        "time-stamp", 0, POPT_ARG_STRING, nullptr, OPT_TIME_STAMP,
        T_("Use TIMESTAMP (elapsed seconds since 1970) instead of current time."),
        T_("TIMESTAMP")
    },
    {
        "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
        T_("Show version information and exit."),
        nullptr
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

void FatalError(const string& msg)
{
    cerr << Utils::GetExeName() << ": " << msg << endl;
    throw 1;
}

void Main(int argc, const char** argv)
{
    int option;
    time_t timeStamp = time(nullptr);
    PoptWrapper popt(argc, argv, aoption);
    popt.SetOtherOptionHelp(T_("[OPTION...] [FILE...]"));
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_TIME_STAMP:
            timeStamp = static_cast<time_t>(std::stoll(popt.GetOptArg()));
            break;
        case OPT_VERSION:
            cout
                << Utils::MakeProgramVersionString(Utils::GetExeName().c_str(), VersionNumber(MIKTEX_COMP_MAJOR_VERSION, MIKTEX_COMP_MINOR_VERSION, MIKTEX_COMP_PATCH_VERSION, 0)) << "\n"
                << "Copyright (C) 2008-2022 Christian Schenk" << "\n"
                << "This is free software; see the source for copying conditions.  There is NO" << "\n"
                << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
            return;
        }
    }
    if (option != -1)
    {
        string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
        msg += ": ";
        msg += popt.Strerror(option);
        FatalError(msg);
    }
    vector<string> leftovers = popt.GetLeftovers();
    if (leftovers.empty())
    {
        FatalError(T_("no file name arguments"));
    }
    for (const string& fileName : leftovers)
    {
        if (Directory::Exists(PathName(fileName)))
        {
            Directory::SetTimes(PathName(fileName), timeStamp, timeStamp, timeStamp);
        }
        else
        {
            File::SetTimes(PathName(fileName), timeStamp, timeStamp, timeStamp);
        }
    }
}

int main(int argc, const char** argv)
{
    int exitCode;
    try
    {
        Session::InitInfo initInfo;
        initInfo.SetProgramInvocationName(argv[0]);
        shared_ptr<Session> session = Session::Create(initInfo);
        Main(argc, argv);
        exitCode = 0;
    }
    catch (const MiKTeXException& e)
    {
        Utils::PrintException(e);
        exitCode = 1;
    }
    catch (int r)
    {
        exitCode = r;
    }
    return exitCode;
}
