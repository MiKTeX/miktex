/**
 * @file utilities/filetypes/commands/commands.h
 * @author Christian Schenk
 * @brief filetypes commands
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

namespace OneMiKTeXUtility::Utilities::FileTypes::Commands
{
    std::unique_ptr<OneMiKTeXUtility::Utilities::Command> Register();
    std::unique_ptr<OneMiKTeXUtility::Utilities::Command> Unregister();
}
