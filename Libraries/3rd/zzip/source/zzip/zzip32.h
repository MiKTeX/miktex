/*
 * Author:
 *      Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) Guido Draheim, use under copyleft (LGPL,MPL)
 *
 * The compat32 headers is targeted towards those poor souls who can
 * not compile their application or library with _LARGEFILE_SOURCE but
 * they may still need to access a libzzip compiled in largefile mode.
 * In that case they can change back to calling these suffix32 functions
 * which are ensured to use "long" as the parameter type - just as it used
 * to be before off_t was defined as an alias for it.
 */

#ifndef _ZZIP_ZZIP32_H
#define _ZZIP_ZZIP32_H

#include <zzip/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Scanning files in zip archive.
 * Implemented in zzip/dir.c
 */
_zzip_export
long zzip_telldir32(ZZIP_DIR * dir);
_zzip_export
void zzip_seekdir32(ZZIP_DIR * dir, long offset);

/*
 *  seek and tell functions.
 *  Implemented in zzip/file.c
 */
_zzip_export
long      zzip_seek32(ZZIP_FILE * fp, long offset, int whence);
_zzip_export
long      zzip_tell32(ZZIP_FILE * fp);

#ifdef __cplusplus
}
#endif

#endif
