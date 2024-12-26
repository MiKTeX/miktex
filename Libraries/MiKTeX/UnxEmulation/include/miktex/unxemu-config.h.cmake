/**
 * @file miktex/unxemu-config.h
 * @defgroup Unx emulation
 * @author Christian Schenk
 * @brief Library configuration
 *
 * @copyright Copyright © 2007-2024 Christian Schenk
 *
 * This file is part of the MiKTeX UNXEMU Library.
 *
 * MiKTeX UNXEMU Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <miktex/Definitions.h>

// DLL import/export switch
#if !defined(D2A2BA842ACE40C6A8A17A9358F2147E)
#define MIKTEXUNXEXPORT MIKTEXDLLIMPORT
#endif

// API decoration for exported functions
#define MIKTEXUNXCEEAPI(type) MIKTEXUNXEXPORT type MIKTEXCEECALL

#cmakedefine HAVE_ACCESS 1
#cmakedefine HAVE_ALLOCA 1
#cmakedefine HAVE_CHDIR 1
#cmakedefine HAVE_CHMOD 1
#cmakedefine HAVE_FINITE 1
#cmakedefine HAVE_GETCWD 1
#cmakedefine HAVE_GETPID 1
#cmakedefine HAVE_INDEX 1
#cmakedefine HAVE_MKDIR 1
#cmakedefine HAVE_MKSTEMP 1
#cmakedefine HAVE_OPEN 1
#cmakedefine HAVE_PCLOSE 1
#cmakedefine HAVE_POPEN 1
#cmakedefine HAVE_RENAME 1
#cmakedefine HAVE_RINDEX 1
#cmakedefine HAVE_RMDIR 1
#cmakedefine HAVE_STAT 1
#cmakedefine HAVE_UNLINK 1
#cmakedefine HAVE_UTIME 1
