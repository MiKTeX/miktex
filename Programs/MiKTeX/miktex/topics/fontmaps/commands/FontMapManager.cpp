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
#include <miktex/Core/Session>
#include <miktex/Core/StreamReader>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/Utils>

#include <miktex/Util/PathName>
#include <miktex/Util/Tokenizer>

#include "internal.h"

#include "FontMapManager.h"

#define BOOLSTR(b) ((b) ? "true" : "false")

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;

const map<string, string> FontMapManager::optionDefaults =
{
    {"LW35", "URWkb"},
    {"dvipdfmDownloadBase14", "true"},
    {"dvipsDownloadBase35", "false"},
    {"dvipsPreferOutline", "true"},
    {"jaEmbed", "haranoaji"},
    {"jaVariant", "-04"},
    {"koEmbed", "baekmuk"},
    {"pdftexDownloadBase14", "true"},
    {"scEmbed", "arphic"},
    {"tcEmbed", "arphic"},
};

const map<string, string> FontMapManager::fileURW =
{
    {"uagd8a.pfb", "a010015l.pfb"},
    {"uagdo8a.pfb", "a010035l.pfb"},
    {"uagk8a.pfb", "a010013l.pfb"},
    {"uagko8a.pfb", "a010033l.pfb"},
    {"ubkd8a.pfb", "b018015l.pfb"},
    {"ubkdi8a.pfb", "b018035l.pfb"},
    {"ubkl8a.pfb", "b018012l.pfb"},
    {"ubkli8a.pfb", "b018032l.pfb"},
    {"ucrb8a.pfb", "n022004l.pfb"},
    {"ucrbo8a.pfb", "n022024l.pfb"},
    {"ucrr8a.pfb", "n022003l.pfb"},
    {"ucrro8a.pfb", "n022023l.pfb"},
    {"uhvb8a.pfb", "n019004l.pfb"},
    {"uhvb8ac.pfb", "n019044l.pfb"},
    {"uhvbo8a.pfb", "n019024l.pfb"},
    {"uhvbo8ac.pfb", "n019064l.pfb"},
    {"uhvr8a.pfb", "n019003l.pfb"},
    {"uhvr8ac.pfb", "n019043l.pfb"},
    {"uhvro8a.pfb", "n019023l.pfb"},
    {"uhvro8ac.pfb", "n019063l.pfb"},
    {"uncb8a.pfb", "c059016l.pfb"},
    {"uncbi8a.pfb", "c059036l.pfb"},
    {"uncr8a.pfb", "c059013l.pfb"},
    {"uncri8a.pfb", "c059033l.pfb"},
    {"uplb8a.pfb", "p052004l.pfb"},
    {"uplbi8a.pfb", "p052024l.pfb"},
    {"uplr8a.pfb", "p052003l.pfb"},
    {"uplri8a.pfb", "p052023l.pfb"},
    {"usyr.pfb", "s050000l.pfb"},
    {"utmb8a.pfb", "n021004l.pfb"},
    {"utmbi8a.pfb", "n021024l.pfb"},
    {"utmr8a.pfb", "n021003l.pfb"},
    {"utmri8a.pfb", "n021023l.pfb"},
    {"uzcmi8a.pfb", "z003034l.pfb"},
    {"uzdr.pfb", "d050000l.pfb"},
};

const map<string, string> FontMapManager::fileADOBE =
{
    {"uagd8a.pfb", "agd_____.pfb"},
    {"uagdo8a.pfb", "agdo____.pfb"},
    {"uagk8a.pfb", "agw_____.pfb"},
    {"uagko8a.pfb", "agwo____.pfb"},
    {"ubkd8a.pfb", "bkd_____.pfb"},
    {"ubkdi8a.pfb", "bkdi____.pfb"},
    {"ubkl8a.pfb", "bkl_____.pfb"},
    {"ubkli8a.pfb", "bkli____.pfb"},
    {"ucrb8a.pfb", "cob_____.pfb"},
    {"ucrbo8a.pfb", "cobo____.pfb"},
    {"ucrr8a.pfb", "com_____.pfb"},
    {"ucrro8a.pfb", "coo_____.pfb"},
    {"uhvb8a.pfb", "hvb_____.pfb"},
    {"uhvb8ac.pfb", "hvnb____.pfb"},
    {"uhvbo8a.pfb", "hvbo____.pfb"},
    {"uhvbo8ac.pfb", "hvnbo___.pfb"},
    {"uhvr8a.pfb", "hv______.pfb"},
    {"uhvr8ac.pfb", "hvn_____.pfb"},
    {"uhvro8a.pfb", "hvo_____.pfb"},
    {"uhvro8ac.pfb", "hvno____.pfb"},
    {"uncb8a.pfb", "ncb_____.pfb"},
    {"uncbi8a.pfb", "ncbi____.pfb"},
    {"uncr8a.pfb", "ncr_____.pfb"},
    {"uncri8a.pfb", "nci_____.pfb"},
    {"uplb8a.pfb", "pob_____.pfb"},
    {"uplbi8a.pfb", "pobi____.pfb"},
    {"uplr8a.pfb", "por_____.pfb"},
    {"uplri8a.pfb", "poi_____.pfb"},
    {"usyr.pfb", "sy______.pfb"},
    {"utmb8a.pfb", "tib_____.pfb"},
    {"utmbi8a.pfb", "tibi____.pfb"},
    {"utmr8a.pfb", "tir_____.pfb"},
    {"utmri8a.pfb", "tii_____.pfb"},
    {"uzcmi8a.pfb", "zcmi____.pfb"},
    {"uzdr.pfb", "zd______.pfb"},
};

const map<string, string> FontMapManager::fileADOBEkb =
{
    {"uagd8a.pfb", "pagd8a.pfb"},
    {"uagdo8a.pfb", "pagdo8a.pfb"},
    {"uagk8a.pfb", "pagk8a.pfb"},
    {"uagko8a.pfb", "pagko8a.pfb"},
    {"ubkd8a.pfb", "pbkd8a.pfb"},
    {"ubkdi8a.pfb", "pbkdi8a.pfb"},
    {"ubkl8a.pfb", "pbkl8a.pfb"},
    {"ubkli8a.pfb", "pbkli8a.pfb"},
    {"ucrb8a.pfb", "pcrb8a.pfb"},
    {"ucrbo8a.pfb", "pcrbo8a.pfb"},
    {"ucrr8a.pfb", "pcrr8a.pfb"},
    {"ucrro8a.pfb", "pcrro8a.pfb"},
    {"uhvb8a.pfb", "phvb8a.pfb"},
    {"uhvb8ac.pfb", "phvb8an.pfb"},
    {"uhvbo8a.pfb", "phvbo8a.pfb"},
    {"uhvbo8ac.pfb", "phvbo8an.pfb"},
    {"uhvr8a.pfb", "phvr8a.pfb"},
    {"uhvr8ac.pfb", "phvr8an.pfb"},
    {"uhvro8a.pfb", "phvro8a.pfb"},
    {"uhvro8ac.pfb", "phvro8an.pfb"},
    {"uncb8a.pfb", "pncb8a.pfb"},
    {"uncbi8a.pfb", "pncbi8a.pfb"},
    {"uncr8a.pfb", "pncr8a.pfb"},
    {"uncri8a.pfb", "pncri8a.pfb"},
    {"uplb8a.pfb", "pplb8a.pfb"},
    {"uplbi8a.pfb", "pplbi8a.pfb"},
    {"uplr8a.pfb", "pplr8a.pfb"},
    {"uplri8a.pfb", "pplri8a.pfb"},
    {"usyr.pfb", "psyr.pfb"},
    {"utmb8a.pfb", "ptmb8a.pfb"},
    {"utmbi8a.pfb", "ptmbi8a.pfb"},
    {"utmr8a.pfb", "ptmr8a.pfb"},
    {"utmri8a.pfb", "ptmri8a.pfb"},
    {"uzcmi8a.pfb", "pzcmi8a.pfb"},
    {"uzdr.pfb", "pzdr.pfb"},
};

const map<string, string> FontMapManager::psADOBE =
{
    {"URWGothicL-Demi", "AvantGarde-Demi"},
    {"URWGothicL-DemiObli", "AvantGarde-DemiOblique"},
    {"URWGothicL-Book", "AvantGarde-Book"},
    {"URWGothicL-BookObli", "AvantGarde-BookOblique"},
    {"URWBookmanL-DemiBold", "Bookman-Demi"},
    {"URWBookmanL-DemiBoldItal", "Bookman-DemiItalic"},
    {"URWBookmanL-Ligh", "Bookman-Light"},
    {"URWBookmanL-LighItal", "Bookman-LightItalic"},
    {"NimbusMonL-Bold", "Courier-Bold"},
    {"NimbusMonL-BoldObli", "Courier-BoldOblique"},
    {"NimbusMonL-Regu", "Courier"},
    {"NimbusMonL-ReguObli", "Courier-Oblique"},
    {"NimbusSanL-Bold", "Helvetica-Bold"},
    {"NimbusSanL-BoldCond", "Helvetica-Narrow-Bold"},
    {"NimbusSanL-BoldItal", "Helvetica-BoldOblique"},
    {"NimbusSanL-BoldCondItal", "Helvetica-Narrow-BoldOblique"},
    {"NimbusSanL-Regu", "Helvetica"},
    {"NimbusSanL-ReguCond", "Helvetica-Narrow"},
    {"NimbusSanL-ReguItal", "Helvetica-Oblique"},
    {"NimbusSanL-ReguCondItal", "Helvetica-Narrow-Oblique"},
    {"CenturySchL-Bold", "NewCenturySchlbk-Bold"},
    {"CenturySchL-BoldItal", "NewCenturySchlbk-BoldItalic"},
    {"CenturySchL-Roma", "NewCenturySchlbk-Roman"},
    {"CenturySchL-Ital", "NewCenturySchlbk-Italic"},
    {"URWPalladioL-Bold", "Palatino-Bold"},
    {"URWPalladioL-BoldItal", "Palatino-BoldItalic"},
    {"URWPalladioL-Roma", "Palatino-Roman"},
    {"URWPalladioL-Ital", "Palatino-Italic"},
    {"StandardSymL", "Symbol"},
    {"NimbusRomNo9L-Medi", "Times-Bold"},
    {"NimbusRomNo9L-MediItal", "Times-BoldItalic"},
    {"NimbusRomNo9L-Regu", "Times-Roman"},
    {"NimbusRomNo9L-ReguItal", "Times-Italic"},
    {"URWChanceryL-MediItal", "ZapfChancery-MediumItalic"},
    {"Dingbats", "ZapfDingbats"},
};

MIKTEXNORETURN void FontMapManager::CfgError(const string& message)
{
    this->ctx->ui->FatalError(fmt::format(T_("{0}:{1}: {2}"), cfgContext.path, cfgContext.line, message));
}

MIKTEXNORETURN void FontMapManager::MapError(const string& message)
{
    this->ctx->ui->FatalError(fmt::format(T_("{0}:{1}: {2}"), mapContext.path, mapContext.line, message));
}

std::string FontMapManager::Option(const std::string& optionName)
{
    auto it = this->config.options.find(optionName);
    if (it == this->config.options.end())
    {
        this->ctx->ui->FatalError(fmt::format(T_("{0}: unknown configuration option"), optionName));
    }
    return it->second;
}

void FontMapManager::SetOption(const std::string& optionName, const std::string& value)
{
    auto it = this->config.options.find(optionName);
    if (it == this->config.options.end())
    {
        this->ctx->ui->FatalError(fmt::format(T_("{0}: unknown configuration option"), optionName));
    }
    it->second = value;
    auto configFile = this->ctx->session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR MIKTEX_PATH_DIRECTORY_DELIMITER_STRING "updmap.cfg");
    Configuration partialConfiguration;
    if (File::Exists(configFile))
    {
        Configuration dummy = this->config;
        partialConfiguration = this->ParseConfigFile(configFile, dummy);
        auto& v = partialConfiguration.options.find(optionName);
        if (v != partialConfiguration.options.end() && v->second == value)
        {
            return;
        }
    }
    partialConfiguration.options[optionName] = value;
    this->WriteConfigFile(configFile, partialConfiguration);
}

bool FontMapManager::ToBool(const string& value)
{
    if (Utils::EqualsIgnoreCase(value, BOOLSTR(false)))
    {
        return false;
    }
    else if (Utils::EqualsIgnoreCase(value, BOOLSTR(true)))
    {
        return true;
    }
    this->ctx->ui->FatalError(fmt::format(T_("{0}: invalid bool value"), value));
}

NamingConvention FontMapManager::ToNamingConvention(const string& value)
{
    if (Utils::EqualsIgnoreCase(value, "URW"))
    {
        return NamingConvention::URW;
    }
    else if (Utils::EqualsIgnoreCase(value, "URWkb"))
    {
        return NamingConvention::URWkb;
    }
    else if (Utils::EqualsIgnoreCase(value, "ADOBE"))
    {
        return NamingConvention::ADOBE;
    }
    else if (Utils::EqualsIgnoreCase(value, "ADOBEkb"))
    {
        return NamingConvention::ADOBEkb;
    }
    this->ctx->ui->FatalError(fmt::format(T_("{0}: invalid LW35 value"), value));
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

Configuration FontMapManager::ParseConfigFile(const PathName& path, Configuration& mergedConfig)
{
    Verbose(fmt::format(T_("Parsing configuration file {0}..."), Q_(path)));
    StreamReader reader(path);
    cfgContext.path = path;
    cfgContext.line = 0;
    string line;
    Configuration partialConfig;
    while (reader.ReadLine(line))
    {
        ++cfgContext.line;
        string directive;
        string param;
        if (!ParseConfigLine(line, directive, param))
        {
            continue;
        }
        auto it = mergedConfig.options.find(directive);
        if (it != mergedConfig.options.end())
        {
            it->second = param;
            partialConfig.options[directive] = param;
        }
        else if (Utils::EqualsIgnoreCase(directive, "Map"))
        {
            if (param.empty())
            {
                CfgError(T_("missing file name"));
            }
            mergedConfig.mapFiles.insert(param);
            partialConfig.mapFiles.insert(param);
        }
        else if (Utils::EqualsIgnoreCase(directive, "MixedMap"))
        {
            if (param.empty())
            {
                CfgError(T_("missing file name"));
            }
           mergedConfig.mixedMapFiles.insert(param);
           partialConfig.mixedMapFiles.insert(param);
        }
        else if (Utils::EqualsIgnoreCase(directive, "KanjiMap"))
        {
            if (param.empty())
            {
                CfgError(T_("missing file name"));
            }
            mergedConfig.kanjiMapFiles.insert(param);
            partialConfig.kanjiMapFiles.insert(param);
        }
        else
        {
            CfgError(fmt::format(T_("{0}: invalid configuration directive"), directive));
        }
    }
    reader.Close();
    return partialConfig;
}

void FontMapManager::WriteConfigFile(const PathName& path, const Configuration& config)
{
    Verbose(fmt::format(T_("Writing configuration file {0}..."), Q_(path)));
    ofstream writer = File::CreateOutputStream(path, ios_base::binary);
    for (auto o : config.options)
    {
        writer << fmt::format("{0} {1}", o.first, o.second) << endl;
    }
    for (auto f : config.kanjiMapFiles)
    {
        writer << fmt::format("KanjiMap {0}", f) << endl;
    }
    for (auto f : config.mapFiles)
    {
        writer << fmt::format("Map {0}", f) << endl;
    }
    for (auto f : config.mixedMapFiles)
    {
        writer << fmt::format("MixedMap {0}", f) << endl;
    }
    writer.close();
}

void FontMapManager::Init(ApplicationContext& ctx)
{
    this->ctx = &ctx;
    this->config = Configuration();
    this->config.options = this->optionDefaults;
    static const vector<string> configFiles = {
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
                ParseConfigFile(*it, this->config);
            }
        }
    }
    if (this->config.mapFiles.empty())
    {
        this->ctx->ui->FatalError(T_("empty configuration"));
    }
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
        this->ctx->ui->FatalError(fmt::format(T_("{0}: not found"), Q_(fileName)));
    }
    if (!found)
    {
        Verbose(3, fmt::format(T_("Not using font map file {0}"), Q_(fileName)));
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
    Verbose(2, fmt::format(T_("Parsing Dvips font map file {0}..."), Q_(path)));

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
    Verbose(2, fmt::format(T_("Parsing Dvipdfmx font map file {0}..."), Q_(path)));

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
        this->ctx->ui->FatalError(fmt::format(T_("{0}: not found"), MIKTEX_FC_CACHE_EXE));
    }
#else
    if (!Utils::FindProgram("fc-cache", fcCacheExe))
    {
        this->ctx->ui->FatalError(fmt::format(T_("{0}: not found", "fc-cache")));
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

    set<DvipsFontMapEntry> mixedMaps(CatDvipsFontMaps(this->config.mixedMapFiles));
    set<DvipsFontMapEntry> nonMixedMaps(CatDvipsFontMaps(this->config.mapFiles));
    set<DvipdfmxFontMapEntry> kanjiMaps(CatDvipdfmxFontMaps(this->config.kanjiMapFiles));

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
