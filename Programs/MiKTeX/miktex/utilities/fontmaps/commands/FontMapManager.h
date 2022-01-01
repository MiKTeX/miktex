/**
 * @file topic/fontmaps/commands/FontMapManager.h
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

#include <miktex/Core/Utils>

#include <miktex/Util/PathName>

#include "internal.h"

struct FileContext
{
    MiKTeX::Util::PathName path;
    int line = 0;
};

struct DvipdfmxFontMapEntry
{
    std::string texName;
    std::string encName;
    std::string fontFileName;
    std::string options;
    std::string psName;
    std::string fallbackName;
};

inline bool operator<(const DvipdfmxFontMapEntry& lhs, const DvipdfmxFontMapEntry& rhs)
{
  return lhs.texName < rhs.texName;
}

enum class NamingConvention
{
    URW,
    URWkb,
    ADOBE,
    ADOBEkb
};

struct Configuration
{
    std::set<std::string> mapFiles;
    std::set<std::string> kanjiMapFiles;
    std::set<std::string> mixedMapFiles;
    std::map<std::string, std::string> options;
};

class FontMapManager :
    public MiKTeX::Core::IRunProcessCallback
{
public:

    void Init(OneMiKTeXUtility::ApplicationContext& ctx);
    std::string Option(const std::string& optionName);
    void SetOption(const std::string& optionName, const std::string& value);
    void WriteMapFiles(bool force, const std::string& outputDirectory);

private:

    bool ToBool(const std::string& param);

    NamingConvention ToNamingConvention(const std::string& param);

    bool ParseConfigLine(const std::string& line, std::string& directive, std::string& param);

    Configuration ParseConfigFile(const MiKTeX::Util::PathName& path, Configuration& config);

    void WriteConfigFile(const MiKTeX::Util::PathName& path, const Configuration& config);

    bool LocateFontMapFile(const std::string& fileName, MiKTeX::Util::PathName& path, bool mustExist);

    void ReadDvipsFontMapFile(const std::string& fileName, std::set<MiKTeX::Core::DvipsFontMapEntry>& fontMapEntries, bool mustExist);

    void ReadDvipdfmxFontMapFile(const std::string& fileName, std::set<DvipdfmxFontMapEntry>& fontMapEntries, bool mustExist);

    void WriteHeader(std::ostream& writer, const MiKTeX::Util::PathName& fileName);

    MiKTeX::Util::PathName FontMapDirectory(const std::string& relPath);

    void WriteDvipsFontMap(std::ostream& writer, const std::set<MiKTeX::Core::DvipsFontMapEntry>& fontMapEntries);

    void WriteDvipdfmxFontMap(std::ostream& writer, const std::set<DvipdfmxFontMapEntry>& fontMapEntries);

    void WriteDvipsFontMapFile(const MiKTeX::Util::PathName& path, const std::set<MiKTeX::Core::DvipsFontMapEntry>& fontMapEntries1, const std::set<MiKTeX::Core::DvipsFontMapEntry>& fontMapEntries2, const std::set<MiKTeX::Core::DvipsFontMapEntry>& fontMapEntries3, const std::set<MiKTeX::Core::DvipsFontMapEntry>& fontMapEntries4);

    void WriteDvipdfmxFontMapFile(const MiKTeX::Util::PathName& path, const std::set<DvipdfmxFontMapEntry>& fontMapEntries);

    std::set<MiKTeX::Core::DvipsFontMapEntry> CatDvipsFontMaps(const std::set<std::string>& fileNames);

    std::set<DvipdfmxFontMapEntry> CatDvipdfmxFontMaps(const std::set<std::string>& fileNames);

    std::set<MiKTeX::Core::DvipsFontMapEntry> TransformLW35(const std::set<MiKTeX::Core::DvipsFontMapEntry>& fontMapEntries);

    std::set<MiKTeX::Core::DvipsFontMapEntry> DvipdfmxToDvips(const std::set<DvipdfmxFontMapEntry>& dvipdfmxFontMapEntries);

    void TransformFontFileName(const std::map<std::string, std::string>& transMap, MiKTeX::Core::DvipsFontMapEntry& fontMapEntry);

    void TransformPSName(const  std::map< std::string,  std::string>& files, MiKTeX::Core::DvipsFontMapEntry& fontMapEntry);

    void CopyFile(const MiKTeX::Util::PathName& pathSrc, const MiKTeX::Util::PathName& pathDest);

    void SymlinkOrCopyFiles();

    void BuildFontconfigCache(bool force);

    void CreateFontconfigLocalfontsConf();

    void Verbose(int level, const std::string& s)
    {
        ctx->ui->Verbose(level, s);
    }

    void Verbose(const std::string& s)
    {
        Verbose(1, s);
    }

    MIKTEXNORETURN void CfgError(const  std::string& s);

    MIKTEXNORETURN void MapError(const  std::string& s);

    void ParseDvipsFontMapFile(const MiKTeX::Util::PathName& mapFile,  std::set<MiKTeX::Core::DvipsFontMapEntry>& fontMapEntries);

    void ParseDvipdfmxFontMapFile(const MiKTeX::Util::PathName& mapFile,  std::set<DvipdfmxFontMapEntry>& fontMapEntries);

    bool ParseDvipdfmxFontMapLine(const std::string& line,  DvipdfmxFontMapEntry& fontMapEntry);

    OneMiKTeXUtility::ApplicationContext* ctx;

    static const std::map<std::string, std::string> optionDefaults;

    Configuration config;

    bool BoolOption(const std::string& optionName)
    {
        return this->ToBool(this->Option(optionName));
    }

    NamingConvention NamingConventionOption(const std::string& optionName)
    {
        return this->ToNamingConvention(this->Option(optionName));
    }

    /**
     * @brief Transform file names from URWkb (berry names) to URW (vendor names).
     * 
     */
    static const std::map<std::string, std::string> fileURW;

    /**
     * @brief Transform file names from URWkb (berry names) to ADOBE (vendor names).
     * 
     */
    static const std::map<std::string, std::string> fileADOBE;

    /**
     * @brief Transform file names from URW to ADOBE (both berry names).
     * 
     */
    static const std::map<std::string, std::string> fileADOBEkb;

    /**
     * @brief Transform font names from URW to Adobe.
     * 
     */
    static const std::map<std::string, std::string> psADOBE;

    std::string outputDirectory;

    FileContext cfgContext;

    FileContext mapContext;

    bool OnProcessOutput(const void* output, size_t n) override;

    std::string currentProcessOutputLine;
};
