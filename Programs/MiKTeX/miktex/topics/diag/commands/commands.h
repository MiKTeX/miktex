/**
 * @file topics/diag/commands/commands.h
 * @author Christian Schenk
 * @brief diag commands
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <memory>

#include "internal.h"

#include "topics/Command.h"

namespace OneMiKTeXUtility::Topics::Diagnostics::Commands
{
    std::unique_ptr<OneMiKTeXUtility::Topics::Command> Collect();
}
