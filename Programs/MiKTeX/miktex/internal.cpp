/**
 * @file internal.cpp
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/StreamReader>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Util/PathName>
#include <miktex/Util/PathNameUtil>
#include <miktex/Util/Tokenizer>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;

string OneMiKTeXUtility::Unescape(const string& s)
{
    stringstream out;
    bool wasEsc = false;
    for (auto ch : s)
    {
        if (wasEsc)
        {
            wasEsc = false;
            switch (ch)
            {
            case '\\':
                out << "\\";
                break;
            case 'n':
                out << "\n";
                break;
            default:
                out << "\\" << ch;
                break;
            }
        }
        else if (ch == '\\')
        {
            wasEsc = true;
        }
        else
        {
            out << ch;
        }
    }
    return out.str();
}

string OneMiKTeXUtility::Format(const string& outputTemplate, const PackageInfo& packageInfo)
{
    auto delim = PathNameUtil::PathNameDelimiter;
    return fmt::format(outputTemplate,
        fmt::arg("archiveFileSize", packageInfo.archiveFileSize), // FIXME
        fmt::arg("copyrightOwner", packageInfo.copyrightOwner),
        fmt::arg("copyrightYear", packageInfo.copyrightYear),
        fmt::arg("creator", packageInfo.creator),
        fmt::arg("ctanPath", packageInfo.ctanPath),
        fmt::arg("description", packageInfo.description),
        fmt::arg("digest", packageInfo.digest),
        fmt::arg("displayName", packageInfo.displayName),
        fmt::arg("docFiles", MiKTeX::Util::StringUtil::Flatten(packageInfo.docFiles, delim)),
        fmt::arg("id", packageInfo.id),
        fmt::arg("isContained", packageInfo.IsContained()),
        fmt::arg("isContainer", packageInfo.IsContainer()),
        fmt::arg("isInstalled", packageInfo.IsInstalled()),
        fmt::arg("isInstalledCommon", packageInfo.IsInstalled(MiKTeX::Core::ConfigurationScope::Common)),
        fmt::arg("isInstalledUser", packageInfo.IsInstalled(MiKTeX::Core::ConfigurationScope::User)),
        fmt::arg("isObsolete", packageInfo.isObsolete),
        fmt::arg("isPureContainer", packageInfo.IsPureContainer()),
        fmt::arg("isRemovable", packageInfo.isRemovable),
        fmt::arg("licenseType", packageInfo.licenseType),
        fmt::arg("numFiles", packageInfo.GetNumFiles()),
        fmt::arg("refCount", packageInfo.GetRefCount()),
        fmt::arg("releaseState", packageInfo.releaseState),
        fmt::arg("requiredBy", MiKTeX::Util::StringUtil::Flatten(packageInfo.requiredBy, delim)),
        fmt::arg("requiredPackages", MiKTeX::Util::StringUtil::Flatten(packageInfo.requiredPackages, delim)),
        fmt::arg("runFiles", MiKTeX::Util::StringUtil::Flatten(packageInfo.runFiles, delim)),
        fmt::arg("size", packageInfo.GetSize()),
        fmt::arg("sizeDocFiles", packageInfo.sizeDocFiles),
        fmt::arg("sizeRunFiles", packageInfo.sizeRunFiles),
        fmt::arg("sizeSourceFiles", packageInfo.sizeSourceFiles),
        fmt::arg("sourceFiles", MiKTeX::Util::StringUtil::Flatten(packageInfo.sourceFiles, delim)),
        fmt::arg("targetSystem", packageInfo.targetSystem),
        fmt::arg("timeInstalled", packageInfo.GetTimeInstalled()),
        fmt::arg("timeInstalledCommon", packageInfo.timeInstalledCommon),
        fmt::arg("timeInstalledUser", packageInfo.timeInstalledUser),
        fmt::arg("timePackaged", packageInfo.timePackaged),
        fmt::arg("title", packageInfo.title),
        fmt::arg("version", packageInfo.version),
        fmt::arg("versionDate", packageInfo.versionDate)
    );
}

void OneMiKTeXUtility::ReadNames(const PathName& path, vector<string>& list)
{
    StreamReader reader(path);
    string line;
    while (reader.ReadLine(line))
    {
        Tokenizer tok(line, " \t\n\r");
        if (tok)
        {
            string name = *tok;
            list.push_back(name);
        }
    }
    reader.Close();
}
