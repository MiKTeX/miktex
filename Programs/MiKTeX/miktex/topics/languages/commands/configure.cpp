/**
 * @file topics/languages/commands/configure.cpp
 * @author Christian Schenk
 * @brief languages configure
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
#include <miktex/Util/PathName>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

namespace
{
    class ConfigureCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Configure LaTeX language definition files");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "configure";
        }

        std::string Synopsis() override
        {
            return "configure";
        }
    };
}

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Languages;

unique_ptr<Command> Commands::Configure()
{
    return make_unique<ConfigureCommand>();
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

int ConfigureCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
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

    ctx.ui->Verbose(1, T_("Creating language.dat, language.dat.lua and language.def..."));

    PathName languageDatPath = ctx.session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_LANGUAGE_DAT);
    ofstream languageDat = File::CreateOutputStream(languageDatPath);

    PathName languageDatLuaPath = ctx.session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_LANGUAGE_DAT_LUA);
    ofstream languageDatLua = File::CreateOutputStream(languageDatLuaPath);

    PathName languageDefPath = ctx.session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_LANGUAGE_DEF);
    ofstream languageDef = File::CreateOutputStream(languageDefPath);

    languageDatLua << "return {" << "\n";
    languageDef << "%% e-TeX V2.2" << "\n";

    for (const LanguageInfo& languageInfo : ctx.session->GetLanguages())
    {
        if (languageInfo.exclude)
        {
            continue;
        }

        PathName loaderPath;
        if (!ctx.session->FindFile(languageInfo.loader, "%r/tex//", loaderPath))
        {
            continue;
        }

        // language.dat
        languageDat << languageInfo.key << " " << languageInfo.loader << "\n";
        for (const string& synonym : StringUtil::Split(languageInfo.synonyms, ','))
        {
            languageDat << "=" << synonym << "\n";
        }

        // language.def
        languageDef << "\\addlanguage{" << languageInfo.key << "}{" << languageInfo.loader << "}{}{" << languageInfo.lefthyphenmin << "}{" << languageInfo.righthyphenmin << "}" << "\n";

        // language.dat.lua
        languageDatLua << "\t['" << languageInfo.key << "'] = {" << "\n";
        languageDatLua << "\t\tloader='" << languageInfo.loader << "'," << "\n";
        languageDatLua << "\t\tlefthyphenmin=" << languageInfo.lefthyphenmin << "," << "\n";
        languageDatLua << "\t\trighthyphenmin=" << languageInfo.righthyphenmin << "," << "\n";
        languageDatLua << "\t\tsynonyms={ ";
        int nSyn = 0;
        for (const string& synonym : StringUtil::Split(languageInfo.synonyms, ','))
        {
            languageDatLua << (nSyn > 0 ? "," : "") << "'" << synonym << "'";
            nSyn++;
        }
        languageDatLua << " }," << "\n";
        languageDatLua << "\t\tpatterns='" << languageInfo.patterns << "'," << "\n";
        languageDatLua << "\t\thyphenation='" << languageInfo.hyphenation << "'," << "\n";
        if (!languageInfo.luaspecial.empty())
        {
            languageDatLua << "\t\tspecial='" << languageInfo.luaspecial << "'," << "\n";
        }
        languageDatLua << "\t}," << "\n";
    }

    languageDatLua << "}" << "\n";

    languageDatLua.close();
    Fndb::Add({ {languageDatLuaPath} });

    languageDef.close();
    Fndb::Add({ {languageDefPath} });

    languageDat.close();
    Fndb::Add({ {languageDatPath} });
    
    return 0;
}
