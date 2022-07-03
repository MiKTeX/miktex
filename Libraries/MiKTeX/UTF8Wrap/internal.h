/**
 * @file internal.h
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 2011-2022 Christian Schenk
 *
 * This file is part of the MiKTeX UTF8Wrap Library.
 *
 * The MiKTeX UTF8Wrap Library is licensed under GNU General Public
 * License version 2 or any later version.
 */

#if defined(MIKTEX_UTF8WRAP_SHARED)
#   define MIKTEXUTF8WRAPEXPORT MIKTEXDLLEXPORT
#else
#   define MIKTEXUTF8WRAPEXPORT
#endif

#define BF56453E041E4B58A0EA455A65DD28B1
#include "miktex/utf8wrap.h"

#define MIKTEXSTATICFUNC(type) static type
#define MIKTEXINTERNALFUNC(type) type
#define MIKTEXINTERNALVAR(type) type
