/**
 * @file topics/formats/commands/commands.h
 * @author Christian Schenk
 * @brief formats commands
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <memory>

#include "internal.h"

#include "topics/Command.h"

namespace OneMiKTeXUtility::Topics::Formats::Commands
{
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Build();
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> List();
}
