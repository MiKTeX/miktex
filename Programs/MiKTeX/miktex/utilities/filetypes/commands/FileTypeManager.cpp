/**
 * @file topic/filetypes/commands/FileTypeManager.h
 * @author Christian Schenk
 * @brief File type manager
 *
 * @copyright Copyright © 2002-2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

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
    "asy", ".asy", "Asymptote File", MIKTEX_ASY_EXE, -2, false, "open", "-cd \"%w\" \"%1\"", nullptr,
    "bib", ".bib", "BibTeX Database", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
    "cls", ".cls", "LaTeX Class", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
    "dtx", ".dtx", "LaTeX Macros", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
    "dvi", ".dvi", "DVI File", MIKTEX_YAP_EXE, 1, false, "open", "/dde", "[open(\"%1\")]",
    "dvi", nullptr, nullptr, MIKTEX_YAP_EXE, INT_MAX, false, "print", "/dde", "[print(\"%1\")]",
    "dvi", nullptr, nullptr, MIKTEX_YAP_EXE, INT_MAX, false, "printto", "/dde", "[printto(\"%1\",\"%2\",\"%3\",\"%4\")]",
    "ltx", ".ltx", "LaTeX Document", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
    "pdf", ".pdf", "PDF File", MIKTEX_TEXWORKS_EXE, INT_MAX, false, "open", "\"%1\"", nullptr,
    "sty", ".sty", "LaTeX Style", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
    "tex", ".tex", "TeX Document", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
};

void FileTypeManager::RegisterShellFileTypes(bool reg)
{
    for (const ShellFileType& sft : shellFileTypes)
    {
        string progId = Utils::MakeProgId(sft.lpszComponent);
        if (reg)
        {
            PathName exe;
            if (sft.lpszExecutable != nullptr && !this->ctx->session->FindFile(sft.lpszExecutable, FileType::EXE, exe))
            {
                ctx->ui->FatalError(fmt::format(T_("{0}: executable not found"), sft.lpszExecutable));
            }
            string command;
            if (sft.lpszExecutable != nullptr && sft.lpszCommandArgs != nullptr)
            {
                command = fmt::format("{0} {1}", exe.ToDos(), sft.lpszCommandArgs);
            }
            string iconPath;
            if (sft.lpszExecutable != nullptr && sft.iconIndex != INT_MAX)
            {
                iconPath = fmt::format("{0},{1}", exe.ToDos(), sft.iconIndex);
            }
            if (sft.lpszUserFriendlyName != nullptr || !iconPath.empty())
            {
                Utils::RegisterShellFileType(progId, sft.lpszUserFriendlyName == nullptr ? "" : sft.lpszUserFriendlyName, iconPath);
            }
            if (sft.lpszVerb != nullptr && (!command.empty() || sft.lpszDdeArgs != nullptr))
            {
                Utils::RegisterShellVerb(progId, sft.lpszVerb, command, sft.lpszDdeArgs == nullptr ? "" : sft.lpszDdeArgs);
            }
            if (sft.lpszExtension != nullptr)
            {
                this->ctx->logger->LogInfo(fmt::format("registering file extension: {0}", sft.lpszExtension));
                Utils::RegisterShellFileAssoc(sft.lpszExtension, progId, sft.takeOwnership);
            }
        }
        else
        {
            Utils::UnregisterShellFileType(progId);
            if (sft.lpszExtension != nullptr)
            {
                this->ctx->logger->LogInfo(fmt::format("unregistering file extension: {0}", sft.lpszExtension));
                Utils::UnregisterShellFileAssoc(sft.lpszExtension, progId);
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
