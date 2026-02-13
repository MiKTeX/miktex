/**
 * @file w2c/c-auto.h
 * @author Christian Schenk
 * @brief Web2C emulation
 *
 * @copyright Copyright © 2010-2026 Christian Schenk
 *
 * This file is part of the MiKTeX W2CEMU Library.
 *
 * MiKTeX W2CEMU Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <miktex/W2C/pre.h>
#include <miktex/Version.h>

#cmakedefine HAVE_DIRENT_H 1
#cmakedefine HAVE_MKSTEMP 1
#cmakedefine HAVE_MKTEMP 1
#cmakedefine HAVE_STRUCT_STAT_ST_MTIM 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_SYS_WAIT_H 1
#cmakedefine HAVE_UNISTD_H 1
