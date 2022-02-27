/**
 * @file topics/repositories/commands/commands.h
 * @author Christian Schenk
 * @brief repositories commands
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

namespace OneMiKTeXUtility::Topics::Repositories::Commands
{
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Check();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Info();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> List();
}
