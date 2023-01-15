/**
 * @file LogFile.cpp
 * @author Christian Schenk
 * @brief Setup log file
 *
 * @copyright Copyright © 2000-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Setup Library.
 *
 * The MiKTeX Setup Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"
#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

void LogFile::Load(const PathName& logFileName)
{
    setupService->ReportLine(fmt::format("loading {0}...", logFileName.ToDisplayString()));
    files.clear();
#if defined(MIKTEX_WINDOWS)
    regValues.clear();
#endif

    if (!File::Exists(logFileName))
    {
        return;
    }

    // open the log file
    StreamReader stream(logFileName);

    // read lines
    string line;
    enum Section { None, Files, HKCU, HKLM };
    Section section = None;
    while (stream.ReadLine(line) && !setupService->IsCancelled())
    {
        size_t cchLine = line.length();
        if (cchLine == 0)
        {
            continue;
        }
        if (line[cchLine - 1] == '\n')
        {
            line[--cchLine] = 0;
        }
        if (line == "[files]")
        {
            section = Files;
            continue;
        }
        else if (line == "[hkcu]")
        {
            section = HKCU;
            continue;
        }
        else if (line == "[hklm]")
        {
            section = HKLM;
            continue;
        }
        switch (section)
        {
        case None:
            break;
        case Files:
            files.insert(PathName(line));
            break;
        case HKCU:
        case HKLM:
        {
#if defined(MIKTEX_WINDOWS)
            Tokenizer tok(line, ";");
            if (!tok)
            {
                continue;
            }
            RegValue regvalue;
            regvalue.hkey = (section == HKCU ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE);
            regvalue.strSubKey = *tok;
            ++tok;
            if (!tok)
            {
                continue;
            }
            regvalue.strValueName = *tok;
            regValues.push_back(regvalue);
#endif
            break;
        }
        }
    }

    stream.Close();
}

#if defined(MIKTEX_WINDOWS)
void LogFile::RemoveStartMenu()
{
    if (setupService->IsCancelled())
    {
        return;
    }
    shared_ptr<Session> session = MIKTEX_SESSION();
    int cidl = (session->IsAdminMode() ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS);
    PathName prefix = Utils::GetFolderPath(cidl, cidl, true);
    RemoveFiles(prefix);
}
#endif

#if defined(MIKTEX_WINDOWS)
void LogFile::RemoveRegistrySettings()
{
    setupService->ReportLine(fmt::format("removing {0} registry values...", regValues.size()));
    for (const RegValue& rv : regValues)
    {
        if (setupService->IsCancelled())
        {
            break;
        }
        HKEY hkeySub;
        if (RegOpenKeyExW(rv.hkey, UW_(rv.strSubKey), 0, KEY_ALL_ACCESS, &hkeySub) != ERROR_SUCCESS)
        {
            continue;
        }
        RegDeleteValueW(hkeySub, UW_(rv.strValueName));
        wchar_t szName[BufferSizes::MaxPath];
        DWORD sizeName = BufferSizes::MaxPath;
        FILETIME fileTime;
        bool del = RegEnumKeyExW(hkeySub, 0, szName, &sizeName, nullptr, nullptr, nullptr, &fileTime) == ERROR_NO_MORE_ITEMS;
        RegCloseKey(hkeySub);
        if (del)
        {
            RegDeleteKeyW(rv.hkey, UW_(rv.strSubKey));
        }
    }
}
#endif

void LogFile::RemoveFiles(const PathName& prefix)
{
    setupService->ReportLine(fmt::format("removing MiKTeX files from {0}...", prefix.ToDisplayString()));
    set<PathName> directories;
    for (const PathName& file : files)
    {
        if (setupService->IsCancelled())
        {
            break;
        }
        if (!prefix.Empty() && !Utils::IsParentDirectoryOf(prefix, file))
        {
            continue;
        }
        if (!((PackageInstallerCallback*)setupService)->OnProgress(MiKTeX::Packages::Notification::RemoveFileStart))
        {
            return;
        }
        PathName pathCurDir = file;
        pathCurDir.RemoveFileSpec();
        directories.insert(pathCurDir);
        if (File::Exists(file))
        {
            File::Delete(file, { FileDeleteOption::TryHard });
        }
        if (!((PackageInstallerCallback*)setupService)->OnProgress(MiKTeX::Packages::Notification::RemoveFileEnd))
        {
            return;
        }
    }
    for (const PathName& dir : directories)
    {
        if (setupService->IsCancelled())
        {
            break;
        }
        if (Directory::Exists(dir))
        {
            Directory::RemoveEmptyDirectoryChain(dir);
        }
    }
}
