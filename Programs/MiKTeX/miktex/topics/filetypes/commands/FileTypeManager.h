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
#include <vector>

#include "internal.h"

struct ShellFileType
{
    std::string component;
    std::string extension;
    std::string displayName;
    std::string executable;
    int iconIndex;
    bool takeOwnership;
    std::string verb;
    std::string commandArgs;
    std::string ddeArgs;
};

class FileTypeManager
{
public:

    void Init(OneMiKTeXUtility::ApplicationContext& ctx);
    void Register();
    std::vector<ShellFileType> ShellFileTypes();
    void Unregister();

private:

    void RegisterShellFileTypes(bool reg);

    OneMiKTeXUtility::ApplicationContext* ctx;

    static const ShellFileType shellFileTypes[];
};
