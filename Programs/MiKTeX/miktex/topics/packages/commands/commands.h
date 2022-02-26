/**
 * @file topics/packages/commands/commands.h
 * @author Christian Schenk
 * @brief packages commands
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <memory>

#include "internal.h"

#include "topics/Command.h"

namespace OneMiKTeXUtility::Topics::Packages::Commands
{
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> CheckUpdate();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> CheckUpgrade();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Info();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Install();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> List();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Remove();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Require();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Update();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> UpdatePackageDatabase();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Upgrade();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Verify();
}
