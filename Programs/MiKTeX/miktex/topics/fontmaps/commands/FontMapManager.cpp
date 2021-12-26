/**
 * @file FontMapManager.cpp
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

#include <iostream>
#include <map>
#include <set>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Core/StreamReader>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/Text>
#include <miktex/Core/Utils>

#include <miktex/Util/PathName>
#include <miktex/Util/Tokenizer>

#include "internal.h"

#include "commands.h"

#include "FontMapManager.h"

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define BOOLSTR(b) ((b) ? "true" : "false")

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;

map<string, string> FontMapManager::fileURW;
map<string, string> FontMapManager::fileADOBE;
map<string, string> FontMapManager::fileADOBEkb;
map<string, string> FontMapManager::psADOBE;

MIKTEXNORETURN void FontMapManager::CfgError(const string& s)
{
    this->ctx->logger->LogFatal(s);
    this->ctx->logger->LogFatal(fmt::format("cfg file: {0}", cfgContext.path));
    this->ctx->logger->LogFatal(fmt::format("line: {0}", cfgContext.line));
    this->ctx->ui->FatalError(T_("Configuration error."));
}

MIKTEXNORETURN void FontMapManager::MapError(const string& s)
{
    this->ctx->logger->LogFatal(s);
    this->ctx->logger->LogFatal(fmt::format("map file: {0}",  mapContext.path));
    this->ctx->logger->LogFatal(fmt::format("line: {0}", mapContext.line));
    this->ctx->ui->FatalError(T_("Map file error."));
}

std::string FontMapManager::Option(const std::string& optionName)
{
    auto it = this->options.find(optionName);
    if (it == this->options.end())
    {
        this->ctx->ui->FatalError(fmt::format(T_("Unknown configuration option: {0}"), optionName));
    }
    return it->second;
}

void FontMapManager::SetOption(const std::string& optionName, const std::string& value)
{
    auto it = this->options.find(optionName);
    if (it == this->options.end())
    {
        this->ctx->ui->FatalError(fmt::format(T_("Unknown configuration option: {0}"), optionName));
    }
    it->second = value;
}

bool FontMapManager::ToBool(const string& param)
{
    if (param.empty())
    {
        CfgError(T_("missing bool value"));
    }
    if (Utils::EqualsIgnoreCase(param, BOOLSTR(false)))
    {
        return false;
    }
    else if (Utils::EqualsIgnoreCase(param, BOOLSTR(true)))
    {
        return true;
    }
    else
    {
        CfgError(T_("invalid bool value"));
    }
}

NamingConvention FontMapManager::ToNamingConvention(const string& param)
{
    if (param.empty())
    {
        CfgError(T_("Missing LW35 value"));
    }
    if (Utils::EqualsIgnoreCase(param, "URW"))
    {
        return NamingConvention::URW;
    }
    else if (Utils::EqualsIgnoreCase(param, "URWkb"))
    {
        return NamingConvention::URWkb;
    }
    else if (Utils::EqualsIgnoreCase(param, "ADOBE"))
    {
        return NamingConvention::ADOBE;
    }
    else if (Utils::EqualsIgnoreCase(param, "ADOBEkb"))
    {
        return NamingConvention::ADOBEkb;
    }
    else
    {
        CfgError(fmt::format(T_("Invalid LW35 value: {0}"), param));
    }
}

bool FontMapManager::ParseConfigLine(const string& line, string& directive, string& param)
{
    if (line.empty() || string("*#;%").find_first_of(line[0]) != string::npos)
    {
        return false;
    }
    Tokenizer tok(line, " \t\n");
    if (!tok)
    {
        return false;
    }
    directive = *tok;
    ++tok;
    if (!tok)
    {
        param = "";
    }
    else
    {
        param = *tok;
    }
    return true;
}

void FontMapManager::ParseConfigFile(const PathName& path)
{
    Verbose(fmt::format(T_("Parsing config file {0}..."), Q_(path)));
    StreamReader reader(path);
    cfgContext.path = path;
    cfgContext.line = 0;
    string line;
    while (reader.ReadLine(line))
    {
        ++cfgContext.line;
        string directive;
        string param;
        if (!ParseConfigLine(line, directive, param))
        {
            continue;
        }
        auto it = this->options.find(directive);
        if (it != this->options.end())
        {
            it->second = param;
        }
        else if (Utils::EqualsIgnoreCase(directive, "Map"))
        {
            if (param.empty())
            {
                CfgError(T_("missing map file name"));
            }
            mapFiles.insert(param);
        }
        else if (Utils::EqualsIgnoreCase(directive, "MixedMap"))
        {
            if (param.empty())
            {
                CfgError(T_("missing map file name"));
            }
            mixedMapFiles.insert(param);
        }
        else if (Utils::EqualsIgnoreCase(directive, "KanjiMap"))
        {
            if (param.empty())
            {
                CfgError(T_("missing map file name"));
            }
            kanjiMapFiles.insert(param);
        }
        else
        {
            CfgError(T_("invalid configuration setting"));
        }
    }
    reader.Close();
}

void FontMapManager::Init(ApplicationContext& ctx)
{
    this->ctx = &ctx;

    bool parsedConfig = false;

    static vector<string> configFiles = {
      MIKTEX_PATH_MKFNTMAP_CFG,
      MIKTEX_PATH_MIKTEX_CONFIG_DIR MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "updmap.cfg",
      MIKTEX_PATH_WEB2C_DIR MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "updmap.cfg"
    };

    for (const string& cfgFile : configFiles)
    {
        vector<PathName> cfgFiles;
        if (this->ctx->session->FindFile(cfgFile, MIKTEX_PATH_TEXMF_PLACEHOLDER, { Session::FindFileOption::All }, cfgFiles))
        {
            for (vector<PathName>::const_reverse_iterator it = cfgFiles.rbegin(); it != cfgFiles.rend(); ++it)
            {
                ParseConfigFile(*it);
                parsedConfig = true;
            }
        }
    }

    if (!parsedConfig)
    {
        this->ctx->ui->FatalError(T_("The config file could not be found."));
    }

    fileURW["uagd8a.pfb"] = "a010015l.pfb";
    fileURW["uagdo8a.pfb"] = "a010035l.pfb";
    fileURW["uagk8a.pfb"] = "a010013l.pfb";
    fileURW["uagko8a.pfb"] = "a010033l.pfb";
    fileURW["ubkd8a.pfb"] = "b018015l.pfb";
    fileURW["ubkdi8a.pfb"] = "b018035l.pfb";
    fileURW["ubkl8a.pfb"] = "b018012l.pfb";
    fileURW["ubkli8a.pfb"] = "b018032l.pfb";
    fileURW["ucrb8a.pfb"] = "n022004l.pfb";
    fileURW["ucrbo8a.pfb"] = "n022024l.pfb";
    fileURW["ucrr8a.pfb"] = "n022003l.pfb";
    fileURW["ucrro8a.pfb"] = "n022023l.pfb";
    fileURW["uhvb8a.pfb"] = "n019004l.pfb";
    fileURW["uhvb8ac.pfb"] = "n019044l.pfb";
    fileURW["uhvbo8a.pfb"] = "n019024l.pfb";
    fileURW["uhvbo8ac.pfb"] = "n019064l.pfb";
    fileURW["uhvr8a.pfb"] = "n019003l.pfb";
    fileURW["uhvr8ac.pfb"] = "n019043l.pfb";
    fileURW["uhvro8a.pfb"] = "n019023l.pfb";
    fileURW["uhvro8ac.pfb"] = "n019063l.pfb";
    fileURW["uncb8a.pfb"] = "c059016l.pfb";
    fileURW["uncbi8a.pfb"] = "c059036l.pfb";
    fileURW["uncr8a.pfb"] = "c059013l.pfb";
    fileURW["uncri8a.pfb"] = "c059033l.pfb";
    fileURW["uplb8a.pfb"] = "p052004l.pfb";
    fileURW["uplbi8a.pfb"] = "p052024l.pfb";
    fileURW["uplr8a.pfb"] = "p052003l.pfb";
    fileURW["uplri8a.pfb"] = "p052023l.pfb";
    fileURW["usyr.pfb"] = "s050000l.pfb";
    fileURW["utmb8a.pfb"] = "n021004l.pfb";
    fileURW["utmbi8a.pfb"] = "n021024l.pfb";
    fileURW["utmr8a.pfb"] = "n021003l.pfb";
    fileURW["utmri8a.pfb"] = "n021023l.pfb";
    fileURW["uzcmi8a.pfb"] = "z003034l.pfb";
    fileURW["uzdr.pfb"] = "d050000l.pfb";

    fileADOBE["uagd8a.pfb"] = "agd_____.pfb";
    fileADOBE["uagdo8a.pfb"] = "agdo____.pfb";
    fileADOBE["uagk8a.pfb"] = "agw_____.pfb";
    fileADOBE["uagko8a.pfb"] = "agwo____.pfb";
    fileADOBE["ubkd8a.pfb"] = "bkd_____.pfb";
    fileADOBE["ubkdi8a.pfb"] = "bkdi____.pfb";
    fileADOBE["ubkl8a.pfb"] = "bkl_____.pfb";
    fileADOBE["ubkli8a.pfb"] = "bkli____.pfb";
    fileADOBE["ucrb8a.pfb"] = "cob_____.pfb";
    fileADOBE["ucrbo8a.pfb"] = "cobo____.pfb";
    fileADOBE["ucrr8a.pfb"] = "com_____.pfb";
    fileADOBE["ucrro8a.pfb"] = "coo_____.pfb";
    fileADOBE["uhvb8a.pfb"] = "hvb_____.pfb";
    fileADOBE["uhvb8ac.pfb"] = "hvnb____.pfb";
    fileADOBE["uhvbo8a.pfb"] = "hvbo____.pfb";
    fileADOBE["uhvbo8ac.pfb"] = "hvnbo___.pfb";
    fileADOBE["uhvr8a.pfb"] = "hv______.pfb";
    fileADOBE["uhvr8ac.pfb"] = "hvn_____.pfb";
    fileADOBE["uhvro8a.pfb"] = "hvo_____.pfb";
    fileADOBE["uhvro8ac.pfb"] = "hvno____.pfb";
    fileADOBE["uncb8a.pfb"] = "ncb_____.pfb";
    fileADOBE["uncbi8a.pfb"] = "ncbi____.pfb";
    fileADOBE["uncr8a.pfb"] = "ncr_____.pfb";
    fileADOBE["uncri8a.pfb"] = "nci_____.pfb";
    fileADOBE["uplb8a.pfb"] = "pob_____.pfb";
    fileADOBE["uplbi8a.pfb"] = "pobi____.pfb";
    fileADOBE["uplr8a.pfb"] = "por_____.pfb";
    fileADOBE["uplri8a.pfb"] = "poi_____.pfb";
    fileADOBE["usyr.pfb"] = "sy______.pfb";
    fileADOBE["utmb8a.pfb"] = "tib_____.pfb";
    fileADOBE["utmbi8a.pfb"] = "tibi____.pfb";
    fileADOBE["utmr8a.pfb"] = "tir_____.pfb";
    fileADOBE["utmri8a.pfb"] = "tii_____.pfb";
    fileADOBE["uzcmi8a.pfb"] = "zcmi____.pfb";
    fileADOBE["uzdr.pfb"] = "zd______.pfb";

    fileADOBEkb["uagd8a.pfb"] = "pagd8a.pfb";
    fileADOBEkb["uagdo8a.pfb"] = "pagdo8a.pfb";
    fileADOBEkb["uagk8a.pfb"] = "pagk8a.pfb";
    fileADOBEkb["uagko8a.pfb"] = "pagko8a.pfb";
    fileADOBEkb["ubkd8a.pfb"] = "pbkd8a.pfb";
    fileADOBEkb["ubkdi8a.pfb"] = "pbkdi8a.pfb";
    fileADOBEkb["ubkl8a.pfb"] = "pbkl8a.pfb";
    fileADOBEkb["ubkli8a.pfb"] = "pbkli8a.pfb";
    fileADOBEkb["ucrb8a.pfb"] = "pcrb8a.pfb";
    fileADOBEkb["ucrbo8a.pfb"] = "pcrbo8a.pfb";
    fileADOBEkb["ucrr8a.pfb"] = "pcrr8a.pfb";
    fileADOBEkb["ucrro8a.pfb"] = "pcrro8a.pfb";
    fileADOBEkb["uhvb8a.pfb"] = "phvb8a.pfb";
    fileADOBEkb["uhvb8ac.pfb"] = "phvb8an.pfb";
    fileADOBEkb["uhvbo8a.pfb"] = "phvbo8a.pfb";
    fileADOBEkb["uhvbo8ac.pfb"] = "phvbo8an.pfb";
    fileADOBEkb["uhvr8a.pfb"] = "phvr8a.pfb";
    fileADOBEkb["uhvr8ac.pfb"] = "phvr8an.pfb";
    fileADOBEkb["uhvro8a.pfb"] = "phvro8a.pfb";
    fileADOBEkb["uhvro8ac.pfb"] = "phvro8an.pfb";
    fileADOBEkb["uncb8a.pfb"] = "pncb8a.pfb";
    fileADOBEkb["uncbi8a.pfb"] = "pncbi8a.pfb";
    fileADOBEkb["uncr8a.pfb"] = "pncr8a.pfb";
    fileADOBEkb["uncri8a.pfb"] = "pncri8a.pfb";
    fileADOBEkb["uplb8a.pfb"] = "pplb8a.pfb";
    fileADOBEkb["uplbi8a.pfb"] = "pplbi8a.pfb";
    fileADOBEkb["uplr8a.pfb"] = "pplr8a.pfb";
    fileADOBEkb["uplri8a.pfb"] = "pplri8a.pfb";
    fileADOBEkb["usyr.pfb"] = "psyr.pfb";
    fileADOBEkb["utmb8a.pfb"] = "ptmb8a.pfb";
    fileADOBEkb["utmbi8a.pfb"] = "ptmbi8a.pfb";
    fileADOBEkb["utmr8a.pfb"] = "ptmr8a.pfb";
    fileADOBEkb["utmri8a.pfb"] = "ptmri8a.pfb";
    fileADOBEkb["uzcmi8a.pfb"] = "pzcmi8a.pfb";
    fileADOBEkb["uzdr.pfb"] = "pzdr.pfb";

    psADOBE["URWGothicL-Demi"] = "AvantGarde-Demi";
    psADOBE["URWGothicL-DemiObli"] = "AvantGarde-DemiOblique";
    psADOBE["URWGothicL-Book"] = "AvantGarde-Book";
    psADOBE["URWGothicL-BookObli"] = "AvantGarde-BookOblique";
    psADOBE["URWBookmanL-DemiBold"] = "Bookman-Demi";
    psADOBE["URWBookmanL-DemiBoldItal"] = "Bookman-DemiItalic";
    psADOBE["URWBookmanL-Ligh"] = "Bookman-Light";
    psADOBE["URWBookmanL-LighItal"] = "Bookman-LightItalic";
    psADOBE["NimbusMonL-Bold"] = "Courier-Bold";
    psADOBE["NimbusMonL-BoldObli"] = "Courier-BoldOblique";
    psADOBE["NimbusMonL-Regu"] = "Courier";
    psADOBE["NimbusMonL-ReguObli"] = "Courier-Oblique";
    psADOBE["NimbusSanL-Bold"] = "Helvetica-Bold";
    psADOBE["NimbusSanL-BoldCond"] = "Helvetica-Narrow-Bold";
    psADOBE["NimbusSanL-BoldItal"] = "Helvetica-BoldOblique";
    psADOBE["NimbusSanL-BoldCondItal"] = "Helvetica-Narrow-BoldOblique";
    psADOBE["NimbusSanL-Regu"] = "Helvetica";
    psADOBE["NimbusSanL-ReguCond"] = "Helvetica-Narrow";
    psADOBE["NimbusSanL-ReguItal"] = "Helvetica-Oblique";
    psADOBE["NimbusSanL-ReguCondItal"] = "Helvetica-Narrow-Oblique";
    psADOBE["CenturySchL-Bold"] = "NewCenturySchlbk-Bold";
    psADOBE["CenturySchL-BoldItal"] = "NewCenturySchlbk-BoldItalic";
    psADOBE["CenturySchL-Roma"] = "NewCenturySchlbk-Roman";
    psADOBE["CenturySchL-Ital"] = "NewCenturySchlbk-Italic";
    psADOBE["URWPalladioL-Bold"] = "Palatino-Bold";
    psADOBE["URWPalladioL-BoldItal"] = "Palatino-BoldItalic";
    psADOBE["URWPalladioL-Roma"] = "Palatino-Roman";
    psADOBE["URWPalladioL-Ital"] = "Palatino-Italic";
    psADOBE["StandardSymL"] = "Symbol";
    psADOBE["NimbusRomNo9L-Medi"] = "Times-Bold";
    psADOBE["NimbusRomNo9L-MediItal"] = "Times-BoldItalic";
    psADOBE["NimbusRomNo9L-Regu"] = "Times-Roman";
    psADOBE["NimbusRomNo9L-ReguItal"] = "Times-Italic";
    psADOBE["URWChanceryL-MediItal"] = "ZapfChancery-MediumItalic";
    psADOBE["Dingbats"] = "ZapfDingbats";
}

bool Replace(string& s, const string& s2, const string& s3)
{
    auto p = s.find(s2);
    if(p == string::npos)
    {
        return false;
    }
    s.replace(p, s2.length(), s3);
    return true;
}

bool FontMapManager::LocateFontMapFile(const string& fileNameTemplate, PathName& path, bool mustExist)
{
    string fileName;
    fileName = fileNameTemplate;
    Replace(fileName, "@jaEmbed@", this->Option("jaEmbed"));
    Replace(fileName, "@jaVariant@", this->Option("jaVariant"));
    Replace(fileName, "@scEmbed@", this->Option("scEmbed"));
    Replace(fileName, "@tcEmbed@", this->Option("tcEmbed"));
    Replace(fileName, "@koEmbed@", this->Option("koEmbed"));
    ctx->installer->EnableInstaller(mustExist);
    bool found = this->ctx->session->FindFile(fileName, FileType::MAP, path);
    ctx->installer->EnableInstaller(true);
    if (!found && mustExist)
    {
        this->ctx->ui->FatalError(fmt::format(T_("Font map file {0} could not be found."), Q_(fileName)));
    }
    if (!found)
    {
        Verbose(3, fmt::format(T_("Not using map file {0}"), Q_(fileName)));
    }
    return found;
}

void FontMapManager::WriteHeader(ostream& writer, const PathName& fileName)
{
    writer
        << "%%% " << T_("DO NOT EDIT THIS FILE! It will be replaced when MiKTeX is updated.") << "\n"
        << "%%% " << T_("Run the following command to edit a local version of this file:") << "\n"
        << "%%%   initexmf --edit-config-file updmap" << endl;
}

void FontMapManager::WriteDvipsFontMap(ostream& writer, const set<DvipsFontMapEntry>& fontMapEntries)
{
    for (const DvipsFontMapEntry& fme : fontMapEntries)
    {
        writer << fmt::format("{} {}", fme.texName, fme.psName);
        if (!fme.specialInstructions.empty())
        {
            writer << fmt::format(" \" {} \"", fme.specialInstructions);
        }
        for (Tokenizer tok(fme.headerList, ";"); tok; ++tok)
        {
            writer << fmt::format(" {}", *tok);
        }
        writer << endl;
    }
}

void FontMapManager::WriteDvipdfmxFontMap(ostream& writer, const set<DvipdfmxFontMapEntry>& fontMapEntries)
{
    for (const DvipdfmxFontMapEntry& fme : fontMapEntries)
    {
        writer << fmt::format("{} {} {}", fme.texName, fme.encName, fme.fontFileName);
        if (!fme.options.empty())
        {
            writer << fmt::format(" {}", fme.options);
        }
        writer << endl;
    }
}

PathName FontMapManager::FontMapDirectory(const string& relPath)
{
    PathName path;
    if (!outputDirectory.empty())
    {
        path = outputDirectory;
    }
    else
    {
        path = this->ctx->session->GetSpecialPath(SpecialPath::DataRoot) / PathName("fonts") / PathName("map") / PathName(relPath);
    }
    if (!Directory::Exists(path))
    {
        Directory::Create(path);
    }
    return path;
}

void FontMapManager::WriteDvipsFontMapFile(const PathName& path, const set<DvipsFontMapEntry>& fontMapEntries1, const set<DvipsFontMapEntry>& fontMapEntries2, const set<DvipsFontMapEntry>& fontMapEntries3, const set<DvipsFontMapEntry>& fontMapEntries4)
{
    Verbose(fmt::format(T_("Writing {0}..."), Q_(path)));
    // TODO: backup old file
    ofstream writer = File::CreateOutputStream(path, ios_base::binary);
    WriteHeader(writer, path);
    set<DvipsFontMapEntry> fontMapEntries = fontMapEntries1;
    fontMapEntries.insert(fontMapEntries2.begin(), fontMapEntries2.end());
    fontMapEntries.insert(fontMapEntries3.begin(), fontMapEntries3.end());
    fontMapEntries.insert(fontMapEntries4.begin(), fontMapEntries4.end());
    WriteDvipsFontMap(writer, fontMapEntries);
    writer.close();
    if (!Fndb::FileExists(path))
    {
        Fndb::Add({ {path} });
    }
}

void FontMapManager::WriteDvipdfmxFontMapFile(const PathName& path, const set<DvipdfmxFontMapEntry>& fontMapEntries)
{
    Verbose(fmt::format(T_("Writing {0}..."), Q_(path)));
    // TODO: backup old file
    ofstream writer = File::CreateOutputStream(path, ios_base::binary);
    WriteHeader(writer, path);
    WriteDvipdfmxFontMap(writer, fontMapEntries);
    writer.close();
    if (!Fndb::FileExists(path))
    {
        Fndb::Add({ {path} });
    }
}

void FontMapManager::ParseDvipsFontMapFile(const PathName& path, set<DvipsFontMapEntry>& fontMapEntries)
{
    Verbose(2, fmt::format(T_("Parsing {0}..."), Q_(path)));

    StreamReader reader(path);

    string line;

    mapContext.path = path;
    mapContext.line = 0;

    while (reader.ReadLine(line))
    {
        ++mapContext.line;
        DvipsFontMapEntry fontMapEntry;
        try
        {
            if (Utils::ParseDvipsFontMapLine(line, fontMapEntry))
            {
                fontMapEntries.insert(fontMapEntry);
            }
        }
        catch (const MiKTeXException& e)
        {
            MapError(e.GetErrorMessage());
        }
    }

    reader.Close();
}

void FontMapManager::ParseDvipdfmxFontMapFile(const PathName& path, set<DvipdfmxFontMapEntry>& fontMapEntries)
{
    Verbose(2, fmt::format(T_("Parsing {0}..."), Q_(path)));

    StreamReader reader(path);

    string line;

    mapContext.path = path;
    mapContext.line = 0;

    while (reader.ReadLine(line))
    {
        ++mapContext.line;
        DvipdfmxFontMapEntry fontMapEntry;
        try
        {
            if (ParseDvipdfmxFontMapLine(line, fontMapEntry))
            {
                fontMapEntries.insert(fontMapEntry);
            }
        }
        catch (const MiKTeXException& e)
        {
            MapError(e.GetErrorMessage());
        }
    }

    reader.Close();
}

void TrimLeft(string& s)
{
    auto p = s.find_first_not_of(" \t");
    if (p == string::npos)
    {
        return;
    }
    s = s.substr(p);
}

void TrimRight(string& s)
{
    auto p = s.find_last_not_of(" \t");
    if (p == string::npos)
    {
        return;
    }
    s = s.substr(0, p + 1);
}

string ParseIdentifier(string& s)
{
    TrimLeft(s);
    const auto p = s.find_first_of(" \t%");
    string id = s.substr(0, p);
    if (p == string::npos)
    {
        s = "";
    }
    else
    {
        s = s.substr(p);
    }
    return id;
}

string ParseOptions(string& s)
{
    TrimLeft(s);
    const auto p = s.find_first_of("%");
    string options = s.substr(0, p);
    if (p == string::npos)
    {
        s = "";
    }
    else
    {
        s = s.substr(p);
    }
    TrimRight(options);
    return options;
}

bool ParseSomething(string& s, const string& something)
{
    auto p = s.rfind(something, 0);
    if (p != 0)
    {
        return false;
    }
    s = s.substr(something.length() + 1);
    return true;
}

bool FontMapManager::ParseDvipdfmxFontMapLine(const string& line,  DvipdfmxFontMapEntry& fontMapEntry)
{
    if (line.empty()
        || line[0] <= ' '
        || line[0] == '*'
        || line[0] == '#'
        || line[0] == ';'
        || line[0] == '%'
        )
    {
        return false;
    }
    string remainder = line;
    TrimLeft(remainder);
    fontMapEntry.texName = ParseIdentifier(remainder);
    if (fontMapEntry.texName.empty())
    {
        return false;
    }
    fontMapEntry.encName = ParseIdentifier(remainder);
    if (fontMapEntry.encName.empty())
    {
        return false;
    }
    fontMapEntry.fontFileName = ParseIdentifier(remainder);
    if (fontMapEntry.fontFileName.empty())
    {
        return false;
    }
    fontMapEntry.options = ParseOptions(remainder);
    TrimLeft(remainder);
    if (ParseSomething(remainder, "%!DVIPSFB"))
    {
        fontMapEntry.fallbackName = ParseIdentifier(remainder);
    }
    else if (ParseSomething(remainder, "%!PS"))
    {
        fontMapEntry.psName = ParseIdentifier(remainder);
    }
    return true;
}

bool MIKTEXTHISCALL FontMapManager::OnProcessOutput(const void* output, size_t n)
{
    const char* text = (const char*)output;
    for (size_t idx = 0; idx < n; ++idx)
    {
        char ch = text[idx];
        if (ch == '\r')
        {
            if (idx < n - 1)
            {
                ++idx;
                ch = text[idx];
            }
        }
        if (ch == '\n')
        {
            this->ctx->logger->LogInfo(currentProcessOutputLine);
            currentProcessOutputLine.clear();
        }
        else
        {
            currentProcessOutputLine += ch;
        }
    }
    return true;
}

void FontMapManager::ReadDvipsFontMapFile(const string& fileName, set<DvipsFontMapEntry>& result, bool mustExist)
{
    PathName path;
    if (!LocateFontMapFile(fileName, path, mustExist))
    {
        return;
    }
    ParseDvipsFontMapFile(path, result);
}

set<DvipsFontMapEntry> FontMapManager::CatDvipsFontMaps(const set<string>& fileNames)
{
    set<DvipsFontMapEntry> result;
    for (const string& fn : fileNames)
    {
        ReadDvipsFontMapFile(fn, result, false);
    }
    return result;
}

void FontMapManager::ReadDvipdfmxFontMapFile(const string& fileName, set<DvipdfmxFontMapEntry>& result, bool mustExist)
{
    PathName path;
    if (!LocateFontMapFile(fileName, path, mustExist))
    {
        return;
    }
    ParseDvipdfmxFontMapFile(path, result);
}

set<DvipdfmxFontMapEntry> FontMapManager::CatDvipdfmxFontMaps(const set<string>& fileNames)
{
    set<DvipdfmxFontMapEntry> result;
    for (const string& fn : fileNames)
    {
        ReadDvipdfmxFontMapFile(fn, result, false);
    }
    return result;
}

void FontMapManager::TransformFontFileName(const map<string, string>& transMap, DvipsFontMapEntry& fontMapEntry)
{
    map<string, string>::const_iterator it = transMap.find(fontMapEntry.fontFile);
    if (it != transMap.end())
    {
        fontMapEntry.fontFile = it->second;
    }
    Tokenizer header(fontMapEntry.headerList, ";");
    fontMapEntry.headerList = "";
    for (; header; ++header)
    {
        if (!fontMapEntry.headerList.empty())
        {
            fontMapEntry.headerList += ';';
        }
        string s = *header;
        const char* lpsz;
        for (lpsz = s.c_str(); *lpsz == '<' || *lpsz == '['; ++lpsz)
        {
            fontMapEntry.headerList += *lpsz;
        }
        it = transMap.find(lpsz);
        if (it == transMap.end())
        {
            fontMapEntry.headerList += lpsz;
        }
        else
        {
            fontMapEntry.headerList += it->second;
        }
    }
}

void FontMapManager::TransformPSName(const map<string, string>& names, DvipsFontMapEntry& fontMapEntry)
{
    map<string, string>::const_iterator it = names.find(fontMapEntry.psName);
    if (it != names.end())
    {
        fontMapEntry.psName = it->second;
    }
}

/**
 * @brief Transform font name and file names according to transformation specified by naming convention.
 * 
 * @param fontMapEntries 
 * @return set<DvipsFontMapEntry> 
 */
set<DvipsFontMapEntry> FontMapManager::TransformLW35(const set<DvipsFontMapEntry>& fontMapEntries)
{
    set<DvipsFontMapEntry> result;
    for (const DvipsFontMapEntry& fme : fontMapEntries)
    {
        DvipsFontMapEntry fontMapEntry = fme;
        switch (this->ToNamingConvention(this->Option("LW35")))
        {
        case NamingConvention::URWkb:
            break;
        case NamingConvention::URW:
            TransformFontFileName(fileURW, fontMapEntry);
            break;
        case NamingConvention::ADOBE:
            TransformPSName(psADOBE, fontMapEntry);
            TransformFontFileName(fileADOBE, fontMapEntry);
            break;
        case NamingConvention::ADOBEkb:
            TransformPSName(psADOBE, fontMapEntry);
            TransformFontFileName(fileADOBEkb, fontMapEntry);
            break;
        }
        result.insert(fontMapEntry);
    }
    return result;
}

/**
 * @brief Transform Dvipdfmx font map entries into Dvips font map entries.
 * 
 * @param fontMapEntries 
 * @return set<DvipsFontMapEntry> 
 */
set<DvipsFontMapEntry> FontMapManager::DvipdfmxToDvips(const set<DvipdfmxFontMapEntry>& dvipdfmxFontMapEntries)
{
    set<DvipsFontMapEntry> result;
    for (const DvipdfmxFontMapEntry& dvipdfmxFME : dvipdfmxFontMapEntries)
    {
        DvipsFontMapEntry dvipsFME;
        dvipsFME.texName = dvipdfmxFME.texName;
        // Special case for pre-defined fallback from Unicode encoded font.
        if (!dvipdfmxFME.fallbackName.empty())
        {
            dvipsFME.psName = dvipdfmxFME.fallbackName;
            if (dvipsFME.psName[0] == '!')
            {
                dvipsFME.psName.erase(0, 1);
            }
            Replace(dvipsFME.psName, ",Bold", "");
            if (!dvipsFME.psName.empty())
            {
                result.insert(dvipsFME);
            }
            continue;            
        }
        // Unicode encoded fonts are not supported unless a fallback font was specified (see above).
        if (dvipdfmxFME.encName == "unicode")
        {
            continue;
        }
        string fname = dvipdfmxFME.fontFileName;
        // Replace supported ",SOMETHING" constructs.
        double italicMax = 0.0;
        if (Replace(fname, ",BoldItalic", ""))
        {
            italicMax = 0.3;
        }
        Replace(fname, ",Bold", "");
        if (Replace(fname, ",Italic", ""))
        {
            italicMax = 0.3;
        }
        // Replace supported "/AJ16" and co. for ptex-fontmaps CID emulation.
        auto p = fname.find('/');
        if (p != string::npos)
        {
            fname.erase(p);
        }
        // Break out if unsupported constructs are found: @ / ,
        if (fname.find_first_of("@/,") != string::npos)
        {
            continue;
        }
        // Remove extension.
        p = fname.find('.');
        if (p != string::npos)
        {
            fname.erase(p);
        }
        // Remove leading '!'.
        if (!fname.empty() && fname[0] == '!')
        {
            fname.erase(0, 1);
        }
        // Remove leading :<number>:
        if (!fname.empty() && fname[0] == ':')
        {
            auto end = fname.find(':', 1);
            if (end == string::npos)
            {
                // TODO: error
                continue;
            }
            fname.erase(0, end);
        }
        dvipsFME.psName = dvipdfmxFME.psName;
        if (dvipsFME.psName.empty())
        {
            dvipsFME.psName = fname;
        }
        dvipsFME.psName += "-" + dvipdfmxFME.encName;
        if (italicMax > 0.0)
        {
            dvipsFME.specialInstructions = fmt::format("\"{0} SlantFont\"", italicMax);
        }
        result.insert(dvipsFME);
    }
    return result;
}

void FontMapManager::CopyFile(const PathName& pathSrc, const PathName& pathDest)
{
    Verbose(fmt::format(T_("Copying {0}"), Q_(pathSrc)));
    Verbose(fmt::format(T_("     to {0}..."), Q_(pathDest)));
    File::Copy(pathSrc, pathDest);
    if (!Fndb::FileExists(pathDest))
    {
        Fndb::Add({ {pathDest} });
    }
}

void FontMapManager::SymlinkOrCopyFiles()
{
    PathName dvipsOutputDir(FontMapDirectory("dvips"));
    PathName pdftexOutputDir(FontMapDirectory("pdftex"));

    PathName pathSrc;

    pathSrc = dvipsOutputDir / PathName(this->ToBool(this->Option("dvipsPreferOutline")) ? "psfonts_t1" : "psfonts_pk");
    pathSrc.AppendExtension(".map");
    CopyFile(pathSrc, PathName(dvipsOutputDir, PathName("psfonts.map")));

    pathSrc = pdftexOutputDir / PathName(this->ToBool(this->Option("pdftexDownloadBase14")) ? "pdftex_dl14" : "pdftex_ndl14");
    pathSrc.AppendExtension(".map");
    CopyFile(pathSrc, PathName(pdftexOutputDir, PathName("pdftex.map")));
}

static const char* const topDirs[] = {
#if 0
    // See: https://github.com/MiKTeX/miktex/issues/940
    "fonts" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "type1",
#endif
    "fonts" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "opentype",
    "fonts" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "truetype",
};

void FontMapManager::BuildFontconfigCache(bool force)
{
#if !defined(USE_SYSTEM_FONTCONFIG)
    this->ctx->session->ConfigureFile(PathName(MIKTEX_PATH_FONTCONFIG_CONFIG_FILE));
#endif
    CreateFontconfigLocalfontsConf();
    PathName fcCacheExe;
#if !defined(USE_SYSTEM_FONTCONFIG)
    if (!this->ctx->session->FindFile(MIKTEX_FC_CACHE_EXE, FileType::EXE, fcCacheExe))
    {
        this->ctx->ui->FatalError(T_("The fc-cache executable could not be found."));
    }
#else
    if (!Utils::FindProgram("fc-cache", fcCacheExe))
    {
        this->ctx->ui->FatalError(T_("The fc-cache executable could not be found."));
    }
#endif
    vector<string> arguments{ fcCacheExe.GetFileNameWithoutExtension().ToString() };
    if (ctx->session->IsAdminMode())
    {
#if !defined(USE_SYSTEM_FONTCONFIG)
        arguments.push_back("--miktex-admin");
#else
        arguments.push_back("--system-only");
#endif
    }
    if (force)
    {
        arguments.push_back("--force");
    }
    for (int n = 0; n < this->ctx->ui->VerbosityLevel(); ++n)
    {
        arguments.push_back("--verbose");
    }
#if !defined(USE_SYSTEM_FONTCONFIG)
    arguments.push_back("--miktex-disable-maintenance");
    arguments.push_back("--miktex-disable-diagnose");
#endif
    this->ctx->logger->LogInfo(fmt::format("running: {0}", CommandLineBuilder(arguments).ToString()));
    Process::Run(fcCacheExe, arguments, this);
}

void FontMapManager::CreateFontconfigLocalfontsConf()
{
    PathName configFile(this->ctx->session->GetSpecialPath(SpecialPath::ConfigRoot));
    configFile /= MIKTEX_PATH_FONTCONFIG_LOCALFONTS_FILE;
    StreamWriter writer(configFile);
    writer.WriteLine("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    writer.WriteLine();
    writer.WriteLine("<!--");
    writer.WriteLine(T_("  DO NOT EDIT THIS FILE! It will be replaced when MiKTeX is updated."));
#if !defined(USE_SYSTEM_FONTCONFIG)
    writer.WriteLine(fmt::format(T_("  Instead, edit the configuration file {0}."), MIKTEX_LOCALFONTS2_CONF));
#endif
    writer.WriteLine("-->");
    writer.WriteLine();
    writer.WriteLine("<fontconfig>");
#if !defined(USE_SYSTEM_FONTCONFIG)
    writer.WriteLine(fmt::format("<include>{}</include>", MIKTEX_LOCALFONTS2_CONF));
#endif
    vector<string> paths;
#if !defined(USE_SYSTEM_FONTCONFIG)
    for (const string& path : this->ctx->session->GetFontDirectories())
    {
        paths.push_back(path);
    }
#endif
    for (unsigned r = 0; r < this->ctx->session->GetNumberOfTEXMFRoots(); ++r)
    {
        PathName root = this->ctx->session->GetRootDirectoryPath(r);
        for (const char* dir : topDirs)
        {
            PathName path = root;
            path /= dir;
            if (Directory::Exists(path))
            {
                paths.push_back(path.GetData());
            }
        }
    }
    for (const string& path : paths)
    {
        writer.WriteLine(fmt::format("<dir>{}</dir>", path));
    }
    writer.WriteLine("</fontconfig>");
    writer.Close();
#if defined(USE_SYSTEM_FONTCONFIG)
    if (this->ctx->session->IsAdminMode())
    {
        PathName confd(MIKTEX_SYSTEM_ETC_FONTS_CONFD_DIR);
        File::CreateLink(configFile, confd / PathName("09-miktex.conf"), { CreateLinkOption::ReplaceExisting, CreateLinkOption::Symbolic });
    }
#endif
#if !defined(USE_SYSTEM_FONTCONFIG)
    configFile.RemoveFileSpec();
    configFile /= MIKTEX_LOCALFONTS2_CONF;
    if (!File::Exists(configFile))
    {
        StreamWriter writer(configFile);
        writer.WriteLine("<?xml version=\"1.0\"?>");
        writer.WriteLine("<fontconfig>");
        writer.WriteLine(T_("<!-- REMOVE THIS LINE"));
        writer.WriteLine(T_("<dir>Your font directory here</dir>"));
        writer.WriteLine(T_("<dir>Your font directory here</dir>"));
        writer.WriteLine(T_("<dir>Your font directory here</dir>"));
        writer.WriteLine(T_("     REMOVE THIS LINE -->"));
        writer.WriteLine("</fontconfig>");
        writer.Close();
    }
#endif
}

bool HasPaintType(const DvipsFontMapEntry& fontMapEntry)
{
    return fontMapEntry.specialInstructions.find("PaintType") != string::npos;
}

/**
 * @brief Remove PaintType due to Sebastian's request.
 * 
 * @param transLW35 
 * @param mixedMapFonts 
 * @param nonMixedMapFonts 
 * @return set<DvipsFontMapEntry> 
 */
set<DvipsFontMapEntry> GeneratePdfTeXFontMap(const set<DvipsFontMapEntry>& transLW35, const set<DvipsFontMapEntry>& mixedMap, const set<DvipsFontMapEntry>& nonMixedMap)
{
    set<DvipsFontMapEntry> result = transLW35;
    result.insert(mixedMap.begin(), mixedMap.end());
    result.insert(nonMixedMap.begin(), nonMixedMap.end());
    set<DvipsFontMapEntry>::iterator it = result.begin();
    while (it != result.end())
    {
        if (HasPaintType(*it))
        {
            it = result.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return result;
}

void FontMapManager::WriteMapFiles(bool force, const string& outputDirectory)
{
    this->outputDirectory = outputDirectory;

    set<DvipsFontMapEntry> dvips35;
    ReadDvipsFontMapFile("dvips35.map", dvips35, true);

    set<DvipsFontMapEntry> pdftex35;
    ReadDvipsFontMapFile("pdftex35.map", pdftex35, true);

    set<DvipsFontMapEntry> ps2pk35;
    ReadDvipsFontMapFile("ps2pk35.map", ps2pk35, true);

    set<DvipsFontMapEntry> transLW35_dvips35(TransformLW35(dvips35));
    set<DvipsFontMapEntry> transLW35_pdftex35(TransformLW35(pdftex35));
    set<DvipsFontMapEntry> transLW35_ps2pk35(TransformLW35(ps2pk35));

    set<DvipsFontMapEntry> mixedMaps(CatDvipsFontMaps(mixedMapFiles));
    set<DvipsFontMapEntry> nonMixedMaps(CatDvipsFontMaps(mapFiles));
    set<DvipdfmxFontMapEntry> kanjiMaps(CatDvipdfmxFontMaps(kanjiMapFiles));

    set<DvipsFontMapEntry> transLW35_dftdvips(TransformLW35(this->ToBool(this->Option("dvipsDownloadBase35")) ? ps2pk35 : dvips35));

    set<DvipsFontMapEntry> fromKanji(DvipdfmxToDvips(kanjiMaps));

    set<DvipsFontMapEntry> empty;

    WriteDvipdfmxFontMapFile(FontMapDirectory("dvipdfmx") / PathName("kanjix.map"), kanjiMaps);
    WriteDvipsFontMapFile(FontMapDirectory("dvips") / PathName("builtin35.map"), transLW35_dvips35, empty, empty, empty);
    WriteDvipsFontMapFile(FontMapDirectory("dvips") / PathName("download35.map"), transLW35_ps2pk35, empty, empty, empty);
    WriteDvipsFontMapFile(FontMapDirectory("dvips") / PathName("ps2pk.map"), transLW35_ps2pk35, mixedMaps, nonMixedMaps, empty);
    WriteDvipsFontMapFile(FontMapDirectory("dvips") / PathName("psfonts_pk.map"), transLW35_dftdvips, empty, nonMixedMaps, fromKanji);
    WriteDvipsFontMapFile(FontMapDirectory("dvips") / PathName("psfonts_t1.map"), transLW35_dftdvips, mixedMaps, nonMixedMaps, fromKanji);
    WriteDvipsFontMapFile(FontMapDirectory("pdftex") / PathName("pdftex_dl14.map"), GeneratePdfTeXFontMap(transLW35_ps2pk35, mixedMaps, nonMixedMaps), empty, empty, empty);
    WriteDvipsFontMapFile(FontMapDirectory("pdftex") / PathName("pdftex_ndl14.map"), GeneratePdfTeXFontMap(transLW35_pdftex35, mixedMaps, nonMixedMaps), empty, empty, empty);

    SymlinkOrCopyFiles();

    BuildFontconfigCache(force);
}

void FontMapManager::WriteConfigurationFile()
{
    // TODO
}
