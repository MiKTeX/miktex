/**
 * @file makefmt.cpp
 * @author Christian Schenk
 * @brief MiKTeX MakeFMT
 *
 * @copyright Copyright © 1998-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Make Utility Collection.
 *
 * The MiKTeX Make Utility Collection is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include "config.h"

#include "makefmt-version.h"

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Util/Tokenizer>

#include "MakeUtility.h"

using namespace std;

using namespace MiKTeX::App;
using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("makefmt"));

enum
{
    OPT_AAA = 1,
    OPT_DESTNAME,
    OPT_ENGINE,
    OPT_ENGINE_OPTION,
    OPT_JOB_TIME,
    OPT_NO_DUMP,
    OPT_PRELOAD,
};

enum class Engine
{
    HiTeX,
    LuaHBTeX,
    LuaTeX,
    TeX,
    XeTeX,
    epTeX,
    eupTeX,
    pdfTeX,
};

class PdfConfigValues :
    public HasNamedValues
{

public:

    bool TryGetValue(const string& valueName, string& value) override
    {
        unordered_map<string, string>::const_iterator it = values.find(valueName);
        if (it == values.end())
        {
            return false;
        }
        value = it->second;
        return true;
    }

    string GetValue(const string& valueName) override
    {
        string value;
        if (!TryGetValue(valueName, value))
        {
            MIKTEX_FATAL_ERROR_2(T_("The pdfTeX configuration file pdftexconfig.tex is incomplete."), "valueName", valueName);
        }
        return value;
    }

    string& operator[](const string& valueName)
    {
        return values[valueName];
    }

private:

    unordered_map<string, string> values;
};

class MakeFmt :
    public MakeUtility
{

public:

    void Run(int argc, const char** argv) override;

    const char* GetEngineName()
    {
        switch (engine)
        {
        case Engine::LuaTeX:
            return "luatex";
        case Engine::TeX:
            return "tex";
        case Engine::HiTeX:
            return "hitex";
        case Engine::pdfTeX:
            return "pdftex";
        case Engine::epTeX:
            return "eptex";
        case Engine::eupTeX:
            return "euptex";
        case Engine::XeTeX:
            return "xetex";
        case Engine::LuaHBTeX:
            return "luahbtex";
        }
        MIKTEX_UNEXPECTED();
    }

private:

    PdfConfigValues ParsePdfConfigFiles() const;
    void CreateDestinationDirectory() override;
    void FindInputFile(const PathName& inputName, PathName& inputFile);
    void InstallPdftexConfigTeX() const;
    void ParsePdfConfigFile(const PathName& cfgFile, PdfConfigValues& values) const;
    void Usage() override;

    void SetEngine(const char* engine)
    {
        if (Utils::EqualsIgnoreCase(engine, "luatex"))
        {
            this->engine = Engine::LuaTeX;
        }
        else if (Utils::EqualsIgnoreCase(engine, "tex"))
        {
            this->engine = Engine::TeX;
        }
        else if (Utils::EqualsIgnoreCase(engine, "hitex"))
        {
            this->engine = Engine::HiTeX;
        }
        else if (Utils::EqualsIgnoreCase(engine, "pdftex"))
        {
            this->engine = Engine::pdfTeX;
        }
        else if (Utils::EqualsIgnoreCase(engine, "eptex"))
        {
            this->engine = Engine::epTeX;
        }
        else if (Utils::EqualsIgnoreCase(engine, "euptex"))
        {
            this->engine = Engine::eupTeX;
        }
        else if (Utils::EqualsIgnoreCase(engine, "xetex"))
        {
            this->engine = Engine::XeTeX;
        }
        else if (Utils::EqualsIgnoreCase(engine, "luahbtex"))
        {
            this->engine = Engine::LuaHBTeX;
        }
        else
        {
            FatalError(fmt::format(T_("Unknown engine: {0}"), engine));
        }
    }

    void AppendEngineOption(const char* option)
    {
        engineOptions.push_back(option);
    }

    const char* GetEngineExeName()
    {
        switch (engine)
        {
        case Engine::LuaTeX:
            return MIKTEX_LUATEX_EXE;
        case Engine::TeX:
            return MIKTEX_TEX_EXE;
        case Engine::HiTeX:
            return MIKTEX_HITEX_EXE;
        case Engine::pdfTeX:
            return MIKTEX_PDFTEX_EXE;
        case Engine::epTeX:
            return MIKTEX_EPTEX_EXE;
        case Engine::eupTeX:
            return MIKTEX_EUPTEX_EXE;
        case Engine::XeTeX:
            return MIKTEX_XETEX_EXE;
        case Engine::LuaHBTeX:
            return MIKTEX_LUAHBTEX_EXE;
        }
        MIKTEX_UNEXPECTED();
    }

    bool IsPdf() const
    {
        bool result = engine == Engine::LuaTeX || engine == Engine::pdfTeX;
        result = result || engine == Engine::LuaHBTeX;
        return result;
    }

    bool IsExtended() const
    {
        bool result = engine == Engine::LuaTeX || engine == Engine::pdfTeX || engine == Engine::XeTeX;
        result = result || engine == Engine::epTeX;
        result = result || engine == Engine::eupTeX;
        result = result || engine == Engine::LuaHBTeX;
        return result;
    }

    BEGIN_OPTION_MAP(MakeFmt)
        OPTION_ENTRY(OPT_ENGINE, SetEngine(optArg))
        OPTION_ENTRY(OPT_ENGINE_OPTION, AppendEngineOption(optArg))
        OPTION_ENTRY_SET(OPT_DESTNAME, destinationName)
        OPTION_ENTRY_SET(OPT_JOB_TIME, jobTime)
        OPTION_ENTRY_SET(OPT_PRELOAD, preloadedFormat)
        OPTION_ENTRY_TRUE(OPT_NO_DUMP, noDumpPrimitive)
    END_OPTION_MAP();

    Engine engine = Engine::TeX;
    PathName destinationName;
    bool noDumpPrimitive = false;
    string jobTime;
    string preloadedFormat;
    vector<string> engineOptions;
};

void MakeFmt::Usage()
{
    cout
        << T_("Usage:") << " " << Utils::GetExeName() << " " << T_("[OPTION]... NAME") << "\n"
        << "\n"
        << T_("This program makes a format file.") << "\n"
        << "\n"
        << T_("NAME is the name of the format, such as 'tex'.") << "\n"
        << "\n"
        << T_("Options:") << "\n"
        << "--debug, -d " << T_("Print debugging information.") << "\n"
        << "--dest-name NAME " << T_("Destination file name.") << "\n"
        << "--disable-installer " << T_("Disable the package installer.") << "\n"
        << "--enable-installer " << T_("Enable the package installer.") << "\n"
        << "--engine=ENGINE " << T_("Set the engine.") << "\n"
        << "--engine-option=OPTION " << T_("Add an engine option.") << "\n"
        << "--help, -h " << T_("Print this help screen and exit.") << "\n"
        << "--job-time=FILE " << T_("Job time is file's modification time.") << "\n"
        << "--no-dump " << T_("Don't issue the \\dump command.") << "\n"
        << "--preload FORMAT " << T_("Format to be preloaded.") << "\n"
        << "--print-only, -n " << T_("Print what commands would be executed.") << "\n"
        << "--verbose, -v " << T_("Print information on what is being done.") << "\n"
        << "--version, -V " << T_("Print the version number and exit.") << "\n";
}

namespace
{
    const struct option aLongOptions[] =
    {
        COMMON_OPTIONS,
        { "dest-name",          required_argument,      nullptr,        OPT_DESTNAME },
        { "engine",             required_argument,      nullptr,        OPT_ENGINE },
        { "engine-option",      required_argument,      nullptr,        OPT_ENGINE_OPTION },
        { "job-time",           required_argument,      nullptr,        OPT_JOB_TIME },
        { "no-dump",            no_argument,            nullptr,        OPT_NO_DUMP },
        { "preload",            required_argument,      nullptr,        OPT_PRELOAD },
        { nullptr,              no_argument,            nullptr,        0 },
    };
}

class CreateDestinationDirectoryCallback :
    public HasNamedValues
{

public:

    bool TryGetValue(const string& valueName, string& value)
    {
        if (valueName == "engine")
        {
            value = parent->GetEngineName();
        }
        else
        {
            return false;
        }
        return true;
    }

    string GetValue(const string& valueName)
    {
        string value;
        if (!TryGetValue(valueName, value))
        {
            MIKTEX_UNEXPECTED();
        }
        return value;
    }

    CreateDestinationDirectoryCallback() = delete;

    CreateDestinationDirectoryCallback(MakeFmt* parent) :
        parent(parent)
    {
    }

private:

    MakeFmt* parent;
};

void MakeFmt::CreateDestinationDirectory()
{
    CreateDestinationDirectoryCallback callback(this);
    destinationDirectory = CreateDirectoryFromTemplate(session->GetConfigValue(MIKTEX_CONFIG_SECTION_MAKEFMT, MIKTEX_CONFIG_VALUE_DESTDIR, &callback).GetString());
}

void MakeFmt::FindInputFile(const PathName& inputName, PathName& inputFile)
{
    if (!session->FindFile(inputName.ToString(), FileType::TEX, inputFile))
    {
        FatalError(fmt::format("The required TeX input file {0} could not be found.", Q_(inputName)));
    }

    LOG4CXX_INFO(logger, "found input file: " << inputFile);

    // remove the directory specification from the path; alternatevly,
    // we could convert backslashes into forward slashes
    inputFile.RemoveDirectorySpec();
}

PdfConfigValues MakeFmt::ParsePdfConfigFiles() const
{
    vector<PathName> cfgFiles;
    if (!session->FindFile(MIKTEX_PATH_PDFTEX_CFG, MIKTEX_PATH_TEXMF_PLACEHOLDER, { Session::FindFileOption::All }, cfgFiles))
    {
        MIKTEX_FATAL_ERROR(T_("The pdfTeX configuration file (pdftex.cfg) could not be found."));
    }
    PdfConfigValues values;
    for (vector<PathName>::const_reverse_iterator it = cfgFiles.rbegin(); it != cfgFiles.rend(); ++it)
    {
        ParsePdfConfigFile(*it, values);
    }
    return values;
}

void MakeFmt::ParsePdfConfigFile(const PathName& cfgFile, PdfConfigValues& values) const
{
    ifstream stream = File::CreateInputStream(cfgFile);
    string line;
    while (std::getline(stream, line))
    {
        Tokenizer tok(line, " \t\r\n");
        if (!tok)
        {
            continue;
        }
        string valueName = *tok;
        tok.SetDelimiters("\r\n");
        ++tok;
        if (!tok)
        {
            continue;
        }
        string value = *tok;
        values[valueName] = value;
    }
}

void MakeFmt::InstallPdftexConfigTeX() const
{
    PdfConfigValues pdfConfigValues = ParsePdfConfigFiles();
    session->ConfigureFile(PathName(MIKTEX_PATH_PDFTEXCONFIG_TEX), &pdfConfigValues);
}

void MakeFmt::Run(int argc, const char** argv)
{
    // get options and file name
    int optionIndex = 0;
    GetOptions(argc, argv, aLongOptions, optionIndex);
    if (argc - optionIndex != 1)
    {
        FatalError(T_("Invalid command-line."));
    }
    name = argv[optionIndex];

    if (destinationName.Empty())
    {
        destinationName = name;
        destinationName.SetExtension("");
    }

    // pretend to be the engine / format
    if (!(destinationName == PathName(GetEngineName())))
    {
        session->PushAppName(GetEngineName());
    }
    session->PushAppName(destinationName.GetData());

    // find the TeX input file
    PathName inputFile;
    FindInputFile(PathName(name), inputFile);

    // create destination directory
    CreateDestinationDirectory();

    // make the format file name
    PathName formatFile(destinationName);
    formatFile.AppendExtension(MIKTEX_FORMAT_FILE_SUFFIX);

    // make fully qualified destination file name
    PathName pathDest(destinationDirectory / destinationName.ToString());
    pathDest.AppendExtension(MIKTEX_FORMAT_FILE_SUFFIX);

    // make the log file name
    PathName logFile(destinationName);
    logFile.AppendExtension(".log");

    // make fully qualified destination log file name
    time_t t = time(nullptr);
    char dateTime[128];
    strftime(dateTime, 128, "%Y-%m-%d-%H-%M-%S", localtime(&t));
    PathName logDest(session->GetSpecialPath(SpecialPath::LogDirectory));
    logDest /= "makefmt";
    logDest /= destinationName.ToString();
    logDest /= dateTime;
    logDest.AppendExtension(".log");

    Verbose(fmt::format(T_("Creating the {0} format file..."), Q_(destinationName)));

    // change to a temporary working directory
    unique_ptr<TemporaryDirectory> wrkDir = TemporaryDirectory::Create();
    wrkDir->SetCurrent();

    // make command line
    bool isHiTeX = engine == Engine::HiTeX;
    vector<string> arguments;
    arguments.push_back("--interaction="s + "nonstopmode");
    if (!isHiTeX)
    {
        arguments.push_back("--initialize");
        arguments.push_back("--halt-on-error");
        if (destinationName != PathName(GetEngineName()))
        {
            arguments.push_back("--alias=" + destinationName.ToString());
        }
        arguments.push_back("--job-name=" + destinationName.ToString());
        if (!jobTime.empty())
        {
            arguments.push_back("--job-time=" + jobTime);
        }
    }
    else
    {
        arguments.push_back("--ini");
        if (destinationName != PathName(GetEngineName()))
        {
            arguments.push_back("--progname=" + destinationName.ToString());
        }
    }
    arguments.insert(arguments.end(), engineOptions.begin(), engineOptions.end());
    if (!preloadedFormat.empty())
    {
        arguments.push_back("&"s + preloadedFormat);
    }
    bool isLuaTeX = engine == Engine::LuaTeX;
    isLuaTeX = isLuaTeX || engine == Engine::LuaHBTeX;
    if (!isLuaTeX && IsExtended() && preloadedFormat.empty())
    {
        arguments.push_back("--enable-etex");
    }
    if (IsPdf())
    {
        InstallPdftexConfigTeX();
    }
    arguments.push_back(inputFile.ToString());
    if (!noDumpPrimitive)
    {
        arguments.push_back("\\dump");
    }

    // run the engine
    bool done = RunProcess(GetEngineExeName(), arguments, wrkDir->GetPathName());

    // install log file
    if (File::Exists(wrkDir->GetPathName() / logFile.ToString()))
    {
        Install(wrkDir->GetPathName() / logFile.ToString(), logDest);
    }

    if (!done)
    {
        FatalError(fmt::format(T_("{0} failed on {1}."), GetEngineExeName(), Q_(name)));
    }

    // install format file
    Install(wrkDir->GetPathName() / formatFile.ToString(), pathDest);
}

#if defined(_UNICODE)
#   define MAIN wmain
#   define MAINCHAR wchar_t
#else
#   define MAIN main
#   define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR** argv)
{
#if defined(MIKTEX_WINDOWS)
    ConsoleCodePageSwitcher cpSwitcher;
#endif
    MakeFmt app;
    try
    {
        vector<string> utf8args;
        utf8args.reserve(argc);
        vector<char*> newargv;
        newargv.reserve(argc + 1);
        for (int idx = 0; idx < argc; ++idx)
        {
#if defined(_UNICODE)
            utf8args.push_back(StringUtil::WideCharToUTF8(argv[idx]));
#elif defined(MIKTEX_WINDOWS)
            utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#else
            utf8args.push_back(argv[idx]);
#endif
            // FIXME: eliminate const cast
            newargv.push_back(const_cast<char*>(utf8args[idx].c_str()));
        }
        newargv.push_back(nullptr);
        app.Init(Session::InitInfo(newargv[0]), newargv);
        app.Run(newargv.size() - 1, const_cast<const char**>(&newargv[0]));
        app.Finalize2(EXIT_SUCCESS);
        logger = nullptr;
        return EXIT_SUCCESS;
    }
    catch (const MiKTeXException& ex)
    {
        ex.Save();
        app.Sorry("makefmt", ex);
        app.Finalize2(EXIT_FAILURE);
        logger = nullptr;
        ex.Save();
        return EXIT_FAILURE;
    }
    catch (const exception& ex)
    {
        app.Sorry("makefmt", ex);
        app.Finalize2(EXIT_FAILURE);
        logger = nullptr;
        return EXIT_FAILURE;
    }
    catch (int exitCode)
    {
        app.Finalize2(exitCode);
        logger = nullptr;
        return exitCode;
    }
}
