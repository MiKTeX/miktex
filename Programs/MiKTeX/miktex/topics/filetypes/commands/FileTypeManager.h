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

#include "internal.h"

struct ShellFileType {
  const char* lpszComponent;
  const char* lpszExtension;
  const char* lpszUserFriendlyName;
  const char* lpszExecutable;
  int iconIndex;
  bool takeOwnership;
  const char* lpszVerb;
  const char* lpszCommandArgs;
  const char* lpszDdeArgs;
};

class FileTypeManager
{
public:

    void Init(OneMiKTeXUtility::ApplicationContext& ctx);
    void Register();
    void Unregister();

private:

    void RegisterShellFileTypes(bool reg);

    OneMiKTeXUtility::ApplicationContext* ctx;

    static const ShellFileType shellFileTypes[];
};
