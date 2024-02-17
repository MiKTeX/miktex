/**
 * @file config.h
 * @author Christian Schenk
 * @brief Library configuration
 *
 * @copyright Copyright © 2008-2024 Christian Schenk
 *
 * This file is part of the MiKTeX KPSEMU Library.
 *
 * MiKTeX KPSEMU Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#cmakedefine HAVE_FSEEKO64 1
#cmakedefine HAVE_FTELLO64 1
#cmakedefine01 WITH_CONTEXT_SUPPORT

#define D0A4167033297F40884B97769F47801C

#if defined(MIKTEX_KPSEMU_SHARED)
#define MIKTEXKPSEXPORT MIKTEXDLLEXPORT
#else
#define MIKTEXKPSEXPORT
#endif
