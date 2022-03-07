/**
 * @file makemf.cpp
 * @author Christian Schenk
 * @brief MiKTeX MakeMF
 *
 * @copyright Copyright © 1998-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Make Utility Collection.
 *
 * The MiKTeX Make Utility Collection is licensed under GNU General Public
 * License version 2 or any later version.
 */

// Some algorithms are borrowed from mktexmf:
// Originally written by Thomas Esser. Public domain.

#include "config.h"

#include "makemf-version.h"

#include <miktex/Core/Fndb>

#include "MakeUtility.h"

using namespace std;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("makemf"));

const char* const lhpref[] =
{
    "la",
    "lb",
    "lc",
    "lh",
    "ll",
    "rx",
    "wn",
    nullptr,
};

const char* const cspref[] =
{
    "cs",
    "ics",
    "lcs",
    nullptr,
};

const char* const cbpref[] =
{
    "glic",
    "glii",
    "glin",
    "glio",
    "gliu",
    "gljc",
    "gljn",
    "gljo",
    "glmc",
    "glmi",
    "glmn",
    "glmo",
    "glmu",
    "gltc",
    "gltn",
    "glto",
    "glwc",
    "glwi",
    "glwn",
    "glwo",
    "glwu",
    "glxc",
    "glxi",
    "glxn",
    "glxo",
    "glxu",
    "gmmn",
    "gmmo",
    "gmtr",
    "gmxn",
    "gmxo",
    "gomc",
    "gomi",
    "gomn",
    "gomo",
    "gomu",
    "goxc",
    "goxi",
    "goxn",
    "goxo",
    "goxu",
    "grbl",
    "grmc",
    "grmi",
    "grml",
    "grmn",
    "grmo",
    "grmu",
    "grxc",
    "grxi",
    "grxl",
    "grxn",
    "grxo",
    "grxu",
    "gsmc",
    "gsmi",
    "gsmn",
    "gsmo",
    "gsmu",
    "gsxc",
    "gsxi",
    "gsxn",
    "gsxo",
    "gsxu",
    "gttc",
    "gtti",
    "gttn",
    "gtto",
    "gttu",
    nullptr,
};

bool HasPrefix(const string& s, const char* const prefixes[])
{
    for (size_t i = 0; prefixes[i] != nullptr; ++i)
    {
        if (HasPrefix(s, prefixes[i]))
        {
            return true;
        }
    }
    return false;
}

class MakeMf :
    public MakeUtility
{

public:

    void Run(int argc, const char** argv) override;

private:

    void CreateDestinationDirectory() override;
    void Usage() override;

    BEGIN_OPTION_MAP(MakeMf)
        OPTION_ENTRY_TRUE('c', toStdout)
    END_OPTION_MAP();

    bool toStdout = false;
    string supplier;
    string typeface;
};

void MakeMf::Usage()
{
    OUT__
        << T_("Usage:") << " " << Utils::GetExeName() << " " << T_("[OPTION]... name") << "\n"
        << "\n"
        << T_("This program makes a METAFONT file.") << "\n"
        << "\n"
        << T_("NAME is the name of the font, such as 'cmr10'.") << "\n"
        << "\n"
        << T_("Options:") << "\n"
        << "--debug, -d " << T_("Print debugging information.") << "\n"
        << "--disable-installer " << T_("Disable the package installer.") << "\n"
        << "--enable-installer " << T_("Enable the package installer.") << "\n"
        << "--help, -h " << T_("Print this help screen and exit.") << "\n"
        << "--print-only, -n " << T_("Print what commands would be executed.") << "\n"
        << "--stdout, -c " << T_("Write MF file on standard output.") << "\n"
        << "--verbose, -v " << T_("Print information on what is being done.") << "\n"
        << "--version, -V " << T_("Print the version number and exit.") << "\n";

}

namespace
{
    const struct option aLongOptions[] =
    {
      COMMON_OPTIONS,
      { "stdout", no_argument, nullptr, 'c' },
      { nullptr, no_argument, nullptr, 0 },
    };
}

void MakeMf::CreateDestinationDirectory()
{
    string templ;
    templ = "%R";
    templ += PathNameUtil::DirectoryDelimiter;
    templ += T_("fonts");
    templ += PathNameUtil::DirectoryDelimiter;
    templ += T_("source");
    if (!supplier.empty())
    {
        templ += PathNameUtil::DirectoryDelimiter;
        templ += supplier;
    }
    templ += PathNameUtil::DirectoryDelimiter;
    templ += typeface;
    destinationDirectory = CreateDirectoryFromTemplate(templ);
}

void MakeMf::Run(int argc, const char** argv)
{
    // get options and file name
    int optionIndex = 0;
    GetOptions(argc, argv, aLongOptions, optionIndex);
    if (argc - optionIndex != 1)
    {
        FatalError(T_("Invalid command-line."));
    }
    name = argv[optionIndex];

    // derive TeX font name from name (e.g., "ecbi3583.mf" =>
    // "ecbi3583")
    string texFontname = PathName(name).GetFileNameWithoutExtension().ToString();

    // derive driver name from the TeX font name (e.g., "ecbi3583" =>
    // "ecbi"
    string driverName;
    session->SplitFontPath(PathName(texFontname), nullptr, nullptr, nullptr, &driverName, nullptr);

    // find the driver file
    {
        // try a sauterized driver first
        string sauterDriverName = "b-";
        sauterDriverName += driverName;
        PathName driverPath;
        if (!session->FindFile(sauterDriverName, FileType::MF, driverPath))
        {
            // lh fonts get special treatment
            if (HasPrefix(driverName, lhpref))
            {
                string lhDriverName;
                lhDriverName += driverName[0];
                lhDriverName += driverName[1];
                lhDriverName += "codes";
                if (!session->FindFile(lhDriverName, FileType::MF, driverPath))
                {
                    FatalError(fmt::format(T_("The {0} source file could not be found."), lhDriverName));
                }
            }
            else if (HasPrefix(driverName, cspref))
            {
                if (!session->FindFile("cscode", FileType::MF, driverPath))
                {
                    FatalError(T_("The cscode source file could not be found."));
                }
            }
            else if (HasPrefix(driverName, cbpref))
            {
                if (!session->FindFile("cbgreek", FileType::MF, driverPath))
                {
                    FatalError(T_("The cbgreek source file could not be found."));
                }
            }
            else if (!session->FindFile(driverName, FileType::MF, driverPath))
            {
                FatalError(fmt::format(T_("The {0} source file could not be found."), driverName));
            }
        }
    }

    // get information about the font
    double true_pt_size;
    if (!session->GetFontInfo(texFontname, supplier, typeface, &true_pt_size))
    {
        FatalError(fmt::format(T_("No info available for {0}."), texFontname));
    }

    // create destination directory
    CreateDestinationDirectory();

    // open the output stream
    ofstream filestream;
    PathName pathDest;
    if (!(toStdout || printOnly))
    {
        // make fully qualified destination file name
        pathDest = destinationDirectory / PathName(texFontname);
        pathDest.AppendExtension(".mf");
        Verbose(fmt::format(T_("Writing on {0}..."), Q_(pathDest)));
        filestream = File::CreateOutputStream(pathDest);
    }
    ostream& stream = toStdout || printOnly ? cout : filestream;

    PrintOnly(fmt::format("cat <<__END__ > {}", Q_(pathDest)));

    if (HasPrefix(texFontname, "ec") || HasPrefix(texFontname, "tc"))
    {
        stream << "if unknown exbase: input exbase fi;" << "\n";
        stream << fmt::format("gensize:={:.2f};", true_pt_size) << "\n";
        stream << fmt::format("generate {};", driverName) << "\n";
    }
    else if (HasPrefix(texFontname, "dc"))
    {
        stream << "if unknown dxbase: input dxbase fi;" << "\n";
        stream << fmt::format("gensize:={:.2f};", true_pt_size) << "\n";
        stream << fmt::format("generate {};", driverName) << "\n";
    }
    else if (HasPrefix(texFontname, lhpref))
    {
        stream << "input fikparm;" << "\n";
    }
    else if (HasPrefix(texFontname, cspref))
    {
        stream << "input cscode\nuse_driver;" << "\n";
    }
    else if (HasPrefix(texFontname, cbpref))
    {
        stream << "input cbgreek;" << "\n";
    }
    else
    {
        stream << fmt::format("design_size:={:.2f};", true_pt_size) << "\n";
        stream << fmt::format("input b-{};", driverName) << "\n";
    }

    PrintOnly("__END__");

    // close output stream
    if (!(toStdout || printOnly))
    {
        filestream.close();

        // add to file name database
        if (!Fndb::FileExists(pathDest))
        {
            Fndb::Add({ {pathDest} });
        }
    }
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
    MakeMf app;
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
        app.Sorry("makemf", ex);
        app.Finalize2(EXIT_FAILURE);
        logger = nullptr;
        return EXIT_FAILURE;
    }
    catch (const exception& ex)
    {
        app.Sorry("makemf", ex);
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
