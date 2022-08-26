/**
 * @file miktex/Configuration/TriState.h
 * @author Christian Schenk
 * @brief Tri state
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Configuration Library.
 *
 * The MiKTeX Configuration Library is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/Configuration/config.h>

MIKTEX_CONFIG_BEGIN_NAMESPACE;

/// Tri-state enum class.
enum class TriState
{
    /// False (no, off, disabled).
    False,
    /// True (yes, on, enabled).
    True,
    /// Undetermined.
    Undetermined,
};

MIKTEX_CONFIG_END_NAMESPACE;
