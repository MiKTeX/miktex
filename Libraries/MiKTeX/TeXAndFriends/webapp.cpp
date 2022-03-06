/**
 * @file webapp.cpp
 * @author Christian Schenk
 * @brief MiKTeX WebApp base implementation
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#include <iostream>
#include <set>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/FileStream>
#include <miktex/Core/VersionNumber>

#include <miktex/Locale/Translator>

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
#include "miktex/TeXAndFriends/WebApp.h"

#include "internal.h"
#include "texmf-version.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Locale;
using namespace MiKTeX::TeXAndFriends;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

class WebApp::impl
{
public:
    impl() = default;
    impl(const impl & other) = delete;
    impl& operator=(const impl & other) = delete;
    impl(impl && other) = delete;
    impl& operator=(impl && other) = delete;
    ~impl() noexcept
    {
        try
        {
            for (char* cstr : cstrings)
            {
                delete[] cstr;
            }
        }
        catch (const exception&)
        {
        }
    }

    char* AddString(const string & s)
    {
        size_t l = s.length();
        char* cstr = new char[l + 1];
        memcpy(cstr, s.c_str(), l + 1);
        cstrings.push_back(cstr);
        return cstr;
    }

    vector<char*> cstrings;
    PoptWrapper popt;
    string copyright;
    PathName packageListFileName;
    C4P::ProgramBase* program;
    string programName;
    string trademarks;
    string version;
    vector<poptOption> options;
    int optBase;
    unordered_map<string, vector<string>> optionShortcuts;
    IInitFinalize* initFinalize = nullptr;
    bool verbose = true;
    static TeXMFResources resources;
    unique_ptr<Translator> translator;
    vector<string> whatIAm;
};

TeXMFResources WebApp::impl::resources;

WebApp::WebApp() :
    pimpl(make_unique<impl>())
{
}

WebApp::~WebApp() noexcept
{
}

void WebApp::Init(vector<char*>& args)
{
    Session::InitInfo initInfo(args[0]);
    initInfo.SetTheNameOfTheGame(TheNameOfTheGame());
    Application::Init(initInfo, args);
    pimpl->translator = make_unique<Translator>(MIKTEX_COMP_ID, &pimpl->resources, GetSession());
    LogInfo(fmt::format("this is MiKTeX-{0} {1} ({2})", pimpl->programName, pimpl->version, Utils::GetMiKTeXBannerString()));
}

void WebApp::Finalize()
{
    shared_ptr<Session> session = GetSession();
    if (!pimpl->packageListFileName.Empty())
    {
        ofstream stream = File::CreateOutputStream(pimpl->packageListFileName);
        vector<FileInfoRecord> fileInfoRecords = session->GetFileInfoRecords();
        set<string> packages;
        for (const FileInfoRecord& fir : fileInfoRecords)
        {
            if (!fir.packageName.empty())
            {
                packages.insert(fir.packageName);
            }
        }
        for (const string& pkg : packages)
        {
            stream << pkg << "\n";
        }
        stream.close();
    }
    pimpl->copyright = "";
    pimpl->packageListFileName = "";
    pimpl->programName = "";
    pimpl->trademarks = "";
    pimpl->version = "";
    pimpl->options.clear();
    pimpl->optionShortcuts.clear();
    pimpl->translator = nullptr;
    Application::Finalize();
}

string WebApp::Translate(const char* msgId)
{
    return pimpl->translator->Translate(msgId);
}

void WebApp::ShowHelp(bool usageOnly) const
{
    if (pimpl->options.empty() || usageOnly || pimpl->popt == nullptr)
    {
        return;
    }
    if (usageOnly)
    {
        pimpl->popt.PrintUsage();
    }
    else
    {
        pimpl->popt.PrintHelp();
    }
}

void WebApp::BadUsage()
{
    cerr << T_("Invalid command-line. Try this:\n") << Utils::GetExeName() << " -help" << endl;
    throw 1;
}

void WebApp::AddOption(const string& name, const string& help, int val, int argInfo, const string& argDescription, void* arg, char shortName)
{
    poptOption opt{};
    opt.longName = pimpl->AddString(name);
    opt.shortName = shortName;
    opt.argInfo = argInfo | POPT_ARGFLAG_ONEDASH;
    if (val == OPT_UNSUPPORTED || val == OPT_NOOP)
    {
        opt.argInfo |= POPT_ARGFLAG_DOC_HIDDEN;
    }
    opt.arg = arg;
    opt.val = val;
    if (!help.empty()
        && val != OPT_UNSUPPORTED
        && val != OPT_NOOP
        && !(argInfo & POPT_ARGFLAG_DOC_HIDDEN))
    {
        opt.descrip = pimpl->AddString(help);
    }
    else
    {
        opt.descrip = nullptr;
    }
    opt.argDescrip = argDescription.empty() ? nullptr : pimpl->AddString(argDescription);
    pimpl->options.push_back(opt);
}

void WebApp::AddOption(const string& aliasName, const string& name)
{
    AddOptionShortcut(aliasName, { "--" + name });
}

void WebApp::AddOptionShortcut(const std::string& longName, const std::vector<std::string>& args)
{
    pimpl->optionShortcuts[longName] = args;
}

enum
{
    OPT_ALIAS,
    OPT_DISABLE_INSTALLER,
    OPT_ENABLE_INSTALLER,
    OPT_HELP,
    OPT_HHELP,
    OPT_INCLUDE_DIRECTORY,
    OPT_RECORD_PACKAGE_USAGES,
    OPT_TRACE,
    OPT_VERBOSE,
    OPT_VERSION,
};

void WebApp::AddOptions()
{
    pimpl->options.reserve(50);
    pimpl->optBase = (int)GetOptions().size();
    AddOption("alias", T_("Pretend to be APP.  This affects both the format used and the search path."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ALIAS, POPT_ARG_STRING, T_("APP"));
    AddOption("disable-installer", T_("Disable the package installer.  Missing files will not be installed."), FIRST_OPTION_VAL + pimpl->optBase + OPT_DISABLE_INSTALLER);
    AddOption("enable-installer", T_("Enable the package installer.  Missing files will be installed."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ENABLE_INSTALLER);
    AddOption("help", T_("Show this help screen and exit."), FIRST_OPTION_VAL + pimpl->optBase + OPT_HELP);
    AddOption("include-directory", T_("Prefix DIR to the input search path."), FIRST_OPTION_VAL + pimpl->optBase + OPT_INCLUDE_DIRECTORY, POPT_ARG_STRING, T_("DIR"));
    AddOption("kpathsea-debug", "", OPT_UNSUPPORTED, POPT_ARG_STRING);
    AddOption("record-package-usages", T_("Enable the package usage recorder.  Output is written to FILE."), FIRST_OPTION_VAL + pimpl->optBase + OPT_RECORD_PACKAGE_USAGES, POPT_ARG_STRING, T_("FILE"));
    AddOption("trace", T_("Turn tracing on.  OPTIONS must be a comma-separated list of trace options.   See the manual, for more information."), FIRST_OPTION_VAL + pimpl->optBase + OPT_TRACE, POPT_ARG_STRING, T_("OPTIONS"));
    AddOption("verbose", T_("Turn on verbose mode."), FIRST_OPTION_VAL + pimpl->optBase + OPT_VERBOSE);
    AddOption("version", T_("Print version information and exit."), FIRST_OPTION_VAL + pimpl->optBase + OPT_VERSION);
#if defined(MIKTEX_WINDOWS)
    if (GetHelpId() > 0)
    {
        AddOption("hhelp", T_("Show the manual page in an HTMLHelp window and exit when the window is closed."), FIRST_OPTION_VAL + pimpl->optBase + OPT_HHELP);
    }
#endif
}

bool WebApp::ProcessOption(int opt, const string& optArg)
{
    shared_ptr<Session> session = GetSession();
    if (opt == OPT_UNSUPPORTED)
    {
        MIKTEX_UNEXPECTED();
    }
    else if (opt == OPT_NOOP)
    {
        return true;
    }
    bool done = true;
    switch (opt - FIRST_OPTION_VAL - pimpl->optBase)
    {
    case OPT_ALIAS:
        session->PushAppName(optArg);
        break;
    case OPT_DISABLE_INSTALLER:
        EnableInstaller(TriState::False);
        break;
    case OPT_ENABLE_INSTALLER:
        EnableInstaller(TriState::True);
        break;
    case OPT_INCLUDE_DIRECTORY:
        if (Directory::Exists(PathName(optArg)))
        {
            PathName path(optArg);
            path.MakeFullyQualified();
            session->AddInputDirectory(path, true);
        }
        break;
    case OPT_HELP:
        ShowHelp();
        throw (0);
#if defined(MIKTEX_WINDOWS)
    case OPT_HHELP:
        MIKTEX_ASSERT(GetHelpId() > 0);
        session->ShowManualPageAndWait(0, GetHelpId());
        throw 0;
#endif
    case OPT_RECORD_PACKAGE_USAGES:
        session->StartFileInfoRecorder(true);
        pimpl->packageListFileName = optArg;
        break;
    case OPT_TRACE:
        MiKTeX::Trace::TraceStream::SetOptions(optArg);
        break;
    case OPT_VERBOSE:
        pimpl->verbose = true;
        break;
    case OPT_VERSION:
        ShowProgramVersion();
        throw 0;
    default:
        done = false;
        break;
    }
    return done;
}

inline bool operator< (const poptOption& opt1, const poptOption& opt2)
{
    MIKTEX_ASSERT(opt1.longName != nullptr);
    MIKTEX_ASSERT(opt2.longName != nullptr);
    return StringCompare(opt1.longName, opt2.longName, false) < 0;
}

void WebApp::ProcessCommandLineOptions()
{
    int argc = GetProgram()->GetArgC();
    const char** argv = GetProgram()->GetArgV();

    if (pimpl->options.empty())
    {
        AddOptions();
        sort(pimpl->options.begin(), pimpl->options.end());
        pimpl->options.push_back(poptOption{});
    }

    pimpl->popt.Construct(argc, argv, &pimpl->options[0]);
    for (auto shortcut : pimpl->optionShortcuts)
    {
        Argv argv(shortcut.second);
        pimpl->popt.AddAlias(shortcut.first.c_str(), 0, argv.GetArgc(), (const char**)argv.CloneFreeable());
    }
    pimpl->popt.SetOtherOptionHelp(GetUsage());

    int opt;

    while ((opt = pimpl->popt.GetNextOpt()) >= 0)
    {
        if (!ProcessOption(opt, pimpl->popt.GetOptArg()))
        {
            MIKTEX_UNEXPECTED();
        }
    }

    if (opt != -1)
    {
        MIKTEX_FATAL_ERROR_2(T_("The command line options could not be processed."), "optionError", pimpl->popt.Strerror(opt));
    }

    GetProgram()->MakeCommandLine(pimpl->popt.GetLeftovers());
}

string WebApp::TheNameOfTheGame() const
{
    return pimpl->programName;
}

void WebApp::ShowProgramVersion() const
{
    cout << "MiKTeX" << '-' << TheNameOfTheGame() << ' ' << VersionNumber(pimpl->version).ToString() << " (" << Utils::GetMiKTeXBannerString() << ')' << endl
        << pimpl->copyright << endl;
    if (!pimpl->trademarks.empty())
    {
        cout << pimpl->trademarks << endl;
    }
    cout << flush;
    ShowLibraryVersions();
}

void WebApp::SetProgram(C4P::ProgramBase* program, const string& programName, const string& version, const string& copyright, const string& trademarks)
{
    pimpl->program = program;
    pimpl->programName = programName;
    pimpl->version = version;
    pimpl->copyright = copyright;
    pimpl->trademarks = trademarks;
}

string WebApp::GetProgramName() const
{
    return pimpl->programName;
}

bool WebApp::AmI(const std::string& name) const
{
    for (auto n : pimpl->whatIAm) 
    {
        if (n == name)
        {
            return true;
        }
    }
    return StringUtil::Contains(GetProgramName().c_str(), name.c_str());
}

void WebApp::IAm(const string& name)
{
    pimpl->whatIAm.push_back(name);
}

void WebApp::SetInitFinalize(IInitFinalize* initFinalize)
{
    pimpl->initFinalize = initFinalize;
}

IInitFinalize* WebApp::GetInitFinalize() const
{
    return pimpl->initFinalize;
}

vector<poptOption> WebApp::GetOptions() const
{
    return pimpl->options;
}

bool WebApp::GetVerboseFlag() const
{
    return pimpl->verbose;
}

C4P::ProgramBase* WebApp::GetProgram() const
{
    return pimpl->program;
}
