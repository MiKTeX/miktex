/**
 * @file topic/filetypes/commands/FileTypeManager.h
 * @author Christian Schenk
 * @brief File type manager
 *
 * @copyright Copyright © 2002-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <config.h>

#include <vector>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Paths>
#include <miktex/Core/Utils>
#include <miktex/Util/PathName>

#include "internal.h"

#include "FileTypeManager.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;

const ShellFileType FileTypeManager::shellFileTypes[] =
{
    "asy", ".asy", "Asymptote File", MIKTEX_ASY_EXE, -2, false, "open", "-cd \"%w\" \"%1\"", "",
    "bib", ".bib", "BibTeX Database", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", "",
    "cls", ".cls", "LaTeX Class", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", "",
    "dtx", ".dtx", "LaTeX Macros", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", "",
    "dvi", ".dvi", "DVI File", MIKTEX_YAP_EXE, 1, false, "open", "/dde", "[open(\"%1\")]",
    "dvi", "", "", MIKTEX_YAP_EXE, INT_MAX, false, "print", "/dde", "[print(\"%1\")]",
    "dvi", "", "", MIKTEX_YAP_EXE, INT_MAX, false, "printto", "/dde", "[printto(\"%1\",\"%2\",\"%3\",\"%4\")]",
    "ltx", ".ltx", "LaTeX Document", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", "",
    "pdf", ".pdf", "PDF File", MIKTEX_TEXWORKS_EXE, INT_MAX, false, "open", "\"%1\"", "",
    "sty", ".sty", "LaTeX Style", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", "",
    "tex", ".tex", "TeX Document", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", "",
};

void FileTypeManager::RegisterShellFileTypes(bool reg)
{
    for (const ShellFileType& sft : this->shellFileTypes)
    {
        string progId = Utils::MakeProgId(sft.component);
        if (reg)
        {
            PathName exe;
            if (!sft.executable.empty() && !this->ctx->session->FindFile(sft.executable, FileType::EXE, exe))
            {
                ctx->ui->FatalError(fmt::format(T_("{0}: executable not found"), sft.executable));
            }
            string command;
            if (!sft.executable.empty() && !sft.commandArgs.empty())
            {
                command = fmt::format("{0} {1}", exe.ToDos(), sft.commandArgs);
            }
            string iconPath;
            if (!sft.executable.empty() && sft.iconIndex != INT_MAX)
            {
                iconPath = fmt::format("{0},{1}", exe.ToDos(), sft.iconIndex);
            }
            if (!sft.displayName.empty() || !iconPath.empty())
            {
                Utils::RegisterShellFileType(progId, sft.displayName, iconPath);
            }
            if (!sft.verb.empty() && (!command.empty() || !sft.ddeArgs.empty()))
            {
                Utils::RegisterShellVerb(progId, sft.verb, command, sft.ddeArgs);
            }
            if (!sft.extension.empty())
            {
                this->ctx->logger->LogInfo(fmt::format("registering file extension: {0}", sft.extension));
                Utils::RegisterShellFileAssoc(sft.extension, progId, sft.takeOwnership);
            }
        }
        else
        {
            Utils::UnregisterShellFileType(progId);
            if (!sft.extension.empty())
            {
                this->ctx->logger->LogInfo(fmt::format("unregistering file extension: {0}", sft.extension));
                Utils::UnregisterShellFileAssoc(sft.extension, progId);
            }
        }
    }
}

void FileTypeManager::Init(ApplicationContext& ctx)
{
    this->ctx = &ctx;
}

void FileTypeManager::Register()
{
    this->RegisterShellFileTypes(true);
}

void FileTypeManager::Unregister()
{
    this->RegisterShellFileTypes(false);
}

vector<ShellFileType> FileTypeManager::ShellFileTypes()
{
    vector<ShellFileType> result;
    for (const ShellFileType& sft : this->shellFileTypes)
    {
        result.push_back(sft);
    }
    return result;
}
