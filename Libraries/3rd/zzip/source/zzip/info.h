/*
 * Author: 
 *      Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) Guido Draheim, use under copyleft (LGPL,MPL)
 */

#ifdef ZZIP_USE_INTERNAL
/* do not make these public, they are for internal use only */

#define ZZIP_error(__dir) __dir->errcode
#define ZZIP_seterror(__dir, __code) __dir->errcode = __code
#define ZZIP_dirhandle(__fp) __fp->dir
#define ZZIP_dirfd(__dir) __dir->fd
#define ZZIP_dir_real(__dir) __dir->realdir != 0
#define ZZIP_file_real(__fp) __fp->dir == 0
#define ZZIP_realdir(__dir) __dir->realdir
#define ZZIP_reafd(__fp) __fp->fd

#endif
