/**
 * @file utilities/formats/commands/commands.h
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

#include "utilities/Command.h"

namespace OneMiKTeXUtility::Utilities::Formats::Commands
{
    std::unique_ptr<OneMiKTeXUtility::Utilities::Command> List();
    std::unique_ptr<OneMiKTeXUtility::Utilities::Command> Update();
}
