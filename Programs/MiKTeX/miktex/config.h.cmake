/**
 * @file config.h
 * @author Christian Schenk
 * @brief Configuration header file
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#cmakedefine USE_SYSTEM_FONTCONFIG 1

#cmakedefine WITH_KPSEWHICH 1
#cmakedefine WITH_MKTEXLSR 1
#cmakedefine WITH_RUNGS 1
#cmakedefine WITH_TEXDOC 1
#cmakedefine WITH_TEXHASH 1
#cmakedefine WITH_TEXLINKS 1
#cmakedefine WITH_UPDMAP 1

#if defined(MIKTEX_MACOS_BUNDLE)
#  define MIKTEX_MACOS_BUNDLE_NAME "${MIKTEX_MACOS_BUNDLE_NAME}"
#endif
