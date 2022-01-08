/**
 * @file topics/languages/commands/commands.h
 * @author Christian Schenk
 * @brief languages commands
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <memory>

#include "internal.h"

#include "topics/Command.h"

namespace OneMiKTeXUtility::Topics::Languages::Commands
{
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Configure();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> List();
}
