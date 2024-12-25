/**
 * @file miktex/TeXAndFriends/Prototypes.h
 * @author Christian Schenk
 * @brief Prototypes
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/TeXAndFriends/config.h>

#include <miktex/Util/PathName>

/// @namespace MiKTeX::TeXAndFriends
///
/// @brief The TeX&Friends namespace.
///
/// This namespace contains C++ classes to support the implementation
/// of TeX (and derived engines), METAFONT, and other TeX releated
/// utilities (e.g., BibTeX).
MIKTEX_TEXMF_BEGIN_NAMESPACE;

MIKTEXMFCEEAPI(bool) OpenOFMFile(void* ptr, const MiKTeX::Util::PathName& fileName);
MIKTEXMFCEEAPI(bool) OpenTFMFile(void* ptr, const MiKTeX::Util::PathName& fileName);
MIKTEXMFCEEAPI(bool) OpenVFFile(void* ptr, const MiKTeX::Util::PathName& fileName);
MIKTEXMFCEEAPI(int) OpenXFMFile(void* ptr, const MiKTeX::Util::PathName& fileName);
MIKTEXMFCEEAPI(int) OpenXVFFile(void* ptr, const MiKTeX::Util::PathName& fileName);

MIKTEX_TEXMF_END_NAMESPACE;
