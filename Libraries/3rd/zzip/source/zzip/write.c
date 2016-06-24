
/*
 * The write-support in zziplib is not a full-flegded interface to the
 * internals that zip file-header or zip archive an contain. It's
 * primary use goes for savegames or transfer `pack-n-go` archives
 * where time-stamps are rather unimportant. Here we can create an 
 * archive with filenames and their data portions, possibly obfuscated.
 *
 * DONT USE THIS
 *
 * The write support is supposed to be added directly into the main
 * zziplib but it has not been implemented so far. It does however
 * export the relevant call entries which will return EROFS (read-only
 * filesystem) in case they are being called. That allows later programs
 * to start up with earlier versions of zziplib that can only read ZIPs.
 *
 * Author: 
 *      Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) 2003 Guido Draheim
 *          All rights reserved,
 *          use under the restrictions of the
 *          Lesser GNU General Public License
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 */

#define _ZZIP_WRITE_SOURCE

#if defined DDDD || defined DDDDD || defined DDDDDD || defined DDDDDDD
#define _ZZIP_ENABLE_WRITE
#else /* per default, we add support for passthrough to posix write */
#define _ZZIP_POSIX_WRITE
#endif

#include <zzip/write.h>         /* #includes <zzip/lib.h> */
#include <zzip/file.h>

#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef ZZIP_HAVE_DIRECT_H
#include <direct.h>
#endif

#include <zzip/format.h>
#include <zzip/plugin.h>
#include <zzip/__debug.h>

#define ___ {
#define ____ }

#ifndef EROFS
# ifdef ENOSYS
#define EROFS ENOSYS
# else
#define EROFS EPERM
#endif
#endif

/* try real zlib routines for writing ? very experimental, very very ex... */
#ifndef _ZZIP_ENABLE_WRITE
#define _ZZIP_TRY 0
#else
#define _ZZIP_TRY 1
#endif

/* btw, is there any system that did define those different ? get away.. */
#  ifndef S_IWGRP
#  define S_IWGRP 00020
#  endif
#  ifndef S_IRWXO
#  define S_IRWXO 00007
#  endif

#  ifdef ZZIP_HAVE_DIRECT_H
#  define _mkdir(a,b) mkdir(a)
#  else
#  define _mkdir      mkdir
#  endif

/** create a new zip archive for writing
 *
 * This function will create a new zip archive. The returned parameter 
 * is a new "zzip dir" handle that should be saved to a variable so it
 * can be used a base argument for => zzip_mkdir and => zzip_creat calls.
 * The returned handle represents a zip central directory that must be
 * saved to disk using => zzip_closedir.
 *
 * Returns null on error and sets errno. Remember, according to posix
 * the => creat(2) call is equivalent to 
   open (path, O_WRONLY | O_CREAT | O_TRUNC, o_mode)
 * so any previous zip-archive will be overwritten unconditionally and
 * EEXIST errors from => mkdir(2) are suppressed. (fixme: delete the
 * given subtree? like suggested by O_TRUNC? not done so far!)
 */
ZZIP_DIR *
zzip_dir_creat(zzip_char_t * name, int o_mode)
{
    return zzip_dir_creat_ext_io(name, o_mode, 0, 0);
}

/** => zzip_dir_creat
 *
 * If the third argument "ext" has another special meaning here, as it
 * is used to ensure that a given zip-file is created with the first entry 
 * of the ext-list appended as an extension unless the file-path already 
 * ends with a file-extension registered in the list. Therefore {"",0} 
 * matches all files and creates them as zip-archives under the given 
 * nonmodified name. (Some magic here? If the path ends in the path
 * separator then make a real directory even in the presence of ext-list?)
 *
 * This function is not yet implemented, check for #def ZZIP_NO_CREAT
 * Write-support will extend => zzip_closedir with semantics to finalize the
 * zip-archive by writing the zip-trailer and closing the archive file.
 */
ZZIP_DIR *
zzip_dir_creat_ext_io(zzip_char_t * name, int o_mode,
                      zzip_strings_t * ext, zzip_plugin_io_t io)
{
    if (! io)
        io = zzip_get_default_io();

    if (io != zzip_get_default_io())
    {
        /* the current io-structure does not contain a "write" entry,
         * and therefore this parameter is useless. Anyone to expect
         * some behavior should be warned, so here we let the function
         * fail bluntly - and leaving the recovery to the application
         */
        errno = EINVAL;
        return 0;
    }


    if (! _ZZIP_TRY)
    {
        /* not implemented - however, we respect that a null argument to 
         * zzip_mkdir and zzip_creat works, so we silently still do the mkdir 
         */
        if (! _mkdir(name, o_mode) || errno == EEXIST)
            errno = EROFS;
        return 0;
    } else
    {
#       define MAX_EXT_LEN 10
        ZZIP_DIR *dir = zzip_dir_alloc(ext);
        int name_len = strlen(name);
        dir->realname = malloc(name_len + MAX_EXT_LEN);
        if (! dir->realname)
            goto error;

        memcpy(dir->realname, name, name_len + 1);
        ___ int fd =
            __zzip_try_open(dir->realname, O_EXCL | O_TRUNC | O_WRONLY, ext,
                            io);
        if (fd != -1)
            { dir->fd = fd; return dir; }

        ___ zzip_strings_t *exx = ext;
        int exx_len;
        for (; *exx; exx++)
        {
            if ((exx_len = strlen(*exx) + 1) <= name_len &&
                ! memcmp(dir->realname + (name_len - exx_len), *exx, exx_len))
                break;          /* keep unmodified */
            exx++;
            if (*exx)
                continue;

            if (! (exx_len = strlen(*exx)) || exx_len >= MAX_EXT_LEN)
                break;
            memcpy(dir->realname + name_len, exx, exx_len);     /* append! */
        }
        ____;
        fd = (io->fd.open)(dir->realname, O_CREAT | O_TRUNC | O_WRONLY, o_mode);
        dir->realname[name_len] = '\0'; /* keep ummodified */
        if (fd != -1)
            { dir->fd = fd; return dir; }
      error:
        zzip_dir_free(dir);
        return 0;
        ____;
    }
}

/** create a new archive area for writing
 *
 * This function will create a new archive area. This may either be a
 * a new zip archive or a new directory in the filesystem. The returned 
 * parameter is a new "zzip dir" handle that should be saved to a variable 
 * so it can be used a base argument for => zzip_file_mkdir and 
 * => zzip_file_creat calls.  The returned handle wraps both possibilities,
 * it can be representing a zip central directory that must be
 * saved to disk using => zzip_closedir or it is just a handle for the
 * name of the real directory that still must be run through 
 * => zzip_closedir to release the wrapper around the directory name.
 *
 * The magic is pushed through the o_mode argument. Using a mode that
 * has no group-write bit set (S_IWGRP = 0040) then the file is
 * created as a zip directory. Note that this is unabridged of any
 * umask value in the system where the argument to this function could
 * be 0775 but with an umask of 0755 it turns out as 0755 for a real
 * directory. Using 0755 directly would not create it as a real directory
 * but as a zip archive handle.
 *
 * This function is not yet implemented, check for #def ZZIP_NO_CREAT
 * Write-support will extend => zzip_closedir with semantics to finalize the
 * zip-archive by writing the zip-trailer and closing the archive file.
 *
 * Returns null on error and sets errno. Remember, according to posix
 * the => creat(2) call is equivalent to 
   open (path, O_WRONLY | O_CREAT | O_TRUNC, o_mode)
 * so any previous zip-archive will be overwritten unconditionally and
 * EEXIST errors from => mkdir(2) are suppressed. (fixme: delete the
 * given subtree? like suggested by O_TRUNC? not done so far!)
 */
ZZIP_DIR *
zzip_createdir(zzip_char_t * name, int o_mode)
{
    if (o_mode & S_IWGRP)
    {
        if (-1 == _mkdir(name, o_mode) && errno != EEXIST)      /* fail */
            return 0;
        return zzip_opendir(name);
    } else
        return zzip_dir_creat(name, o_mode);
}

/** => zzip_file_creat              also: mkdir(2), creat(2), zzip_dir_creat
 *
 * This function has an additional primary argument over the posix
 * mkdir(2) - if it is null then this function behaves just like
 * posix mkdir(2). The zzip_dir argument can be set to the result
 * of a => zzip_createdir which allows for some magic that the
 * given directory name is created as an entry in the zip archive.
 *
 * If the given dir name argument is not within the basepath of 
 * the zip central directory then a real directory is created.
 * Any EEXIST errors are not suppressed unlike with => zzip_createdir
 *
 * Standard usage accepts a global/threaded/modular ZZIP_DIR pointer
 * for all zip archive operations like in:
   ZZIP_DIR* zip = zzip_createdir (sysconfpath, 0755, zip);
   zzip_file_mkdir (zip, filepath[i], 0755);
   ZZIP_FILE* file = zzip_file_creat (zip, filename[i], 0644);
   zzip_write (file, buf, len);
   zzip_close (file); file = 0;
   zzip_closedir (zip); zip = 0;
 *
 * compare with => zzip_mkdir inline macro which allows to
 * collapse the examples script to
   #define zzip_savefile myproject_saveconfig
   #include <zzip/zzip.h>
   ZZIP_DIR* zzip_savefile = zzip_createdir (sysconfpath, 0755);
   zzip_mkdir (filepath[i], 0755);
   ZZIP_FILE* file = zzip_creat(filepath[i], 0644);
   zzip_write (file, buf, len);
   zzip_close (file); file = 0;
   zzip_closedir (zip_savefile);
 */
int
zzip_file_mkdir(ZZIP_DIR * dir, zzip_char_t * name, int o_mode)
{
    if (! dir)
        return _mkdir(name, o_mode);

    if (! _ZZIP_TRY)
    {                           /* not implemented */
        errno = EROFS;
        return -1;
    } else
    {
        errno = EROFS;
        return -1;
    }
}

/** start next file entry in a zip archive
 *
 * This function will create a new file within a zzip archive, the
 * one given as the primary argument and additionally to the posix
 * creat(2) - just like zzip_mkdir has an additional argument over
 * the posix mkdir(2) spec. For this function the primary parameter
 * can be null as well thereby creating a real file instead of a new
 * one inside the zip-archive otherwise given. If the primary parameter is
 * not null but wraps a real directory then all new files are also real.
 *
 * This function is not yet implemented, check for #def ZZIP_NO_CREAT
 *
 * Returns NULL on an error setting errno, and opening a file _within_ 
 * a zip archive using O_RDONLY (and similar stuff) will surely lead to 
 * an error.
 */
ZZIP_FILE *
zzip_file_creat(ZZIP_DIR * dir, zzip_char_t * name, int o_mode)
{
    if (! dir)
        return zzip_open(name, o_mode);

    if (! _ZZIP_TRY)
    {                           /* not implemented */
        errno = EROFS;
        return 0;
    } else
    {
        errno = EROFS;
        return 0;
    }
}

/** write to zzip storage                     also: write(2), zlib(3)
 *
 * This function will write data to a file descriptor. If the file
 * descriptor represents a real file then it will be forwarded to
 * call posix => write(2) directly. If it is a descriptor for a
 * file within a zip directory then the data will be "deflated"
 * using => zlib(3) and appended to the zip archive file.
 */
zzip_ssize_t
zzip_write(ZZIP_FILE * file, const void *ptr, zzip_size_t len)
{
    if (zzip_file_real(file))
        return write(zzip_realfd(file), ptr, len);
    else
        return zzip_file_write(file, ptr, len);
}

/** => zzip_write                            also: zzip_file_creat
 *
 * This function will write data to a file descriptor inside a zip
 * archive. The data will be "deflated" using => zlib(3) compression
 * and appended to the end of the zip archive file. Only one file
 * descriptor may be open per zzip_dir archive handle (fifo-like).
 *
 * This function is not yet implemented, check for #def ZZIP_NO_CREAT
 * It returns immediately -1 and sets errno=EROFS for indication.
 */
zzip_ssize_t
zzip_file_write(ZZIP_FILE * file, const void *ptr, zzip_size_t len)
{
    if (! _ZZIP_TRY)
    {                           /* not implemented */
        errno = EROFS;
        return -1;
    } else
    {
        /* add calls to zlib here... */
        errno = EROFS;
        return -1;
    }
}

/** => zzip_write
 * This function is the stdc variant for writing and the arguments
 * are forwarded to => zzip_write - the return value is floored to
 * null as for STDC spec but there is no zzip_ferror call so far
 * for the zziplib (later? is it actually needed?).
 *
 * This function is not yet implemented, check for #def ZZIP_NO_CREAT
 * Write-support extends => zzip_close with semantics to write out a 
 * file-trailer to the zip-archive leaving a name/offset marker in
 * the (still-open) ZZIP_DIR handle.
 */
zzip_size_t
zzip_fwrite(const void *ptr, zzip_size_t len, zzip_size_t multiply,
            ZZIP_FILE * file)
{
    zzip_ssize_t value = zzip_write(file, ptr, len * multiply);
    if (value == -1)
        value = 0;
    return (zzip_size_t) value;
}

#if 0                           /* pure documentation */

/** create a zipped file/directory            also: zzip_dir_creat, mkdir(2)
 *
 * This function creates a directory entry in the default zip-archive. 
 * If you did  not specify a "#define zzip_savefile somevar" 
 * then the default zip-archive is null and all directories are 
 * created as real directories in the filesystem. This function is 
 * really a preprocessor macro or preferably an inline function
 *  around => zzip_file_mkdir, there is no such symbol generated 
 * into the library. The prototype is modelled after the posix 
 * => mkdir(2) call.
 #ifndef zzip_savefile
 #define zzip_savefile 0
 #endif
 #define zzip_mkdir(name,mode) \ -
         zzip_file_mkdir(zzip_savefile,name,mode)
 *
 */
int inline
zzip_mkdir(zzip_char_t * name, int o_mode)
{
    return zzip_file_creat(zzip_savefile, name, mode);
}
#endif

#if 0                           /* pure documentation */

/** => zzip_mkdir                 also: creat(2), zzip_start
 *
 * This function creates a file in the default zip-archive. 
 * If you did not specify a "#define zzip_savefile somevar" 
 * then the default zip-archive is null and all files are created 
 * as real files. This function is really a preprocessor macro 
 * or preferably an inline function around => zzip_file_creat, 
 * there is no such symbol generated into the library. The prototype
 * is modelled after the posix => creat(2) call.
 #ifndef zzip_savefile
 #define zzip_savefile 0
 #endif
 #define zzip_creat(name,mode) \ -
         zzip_file_creat(zzip_savefile,name,mode)
 */
ZZIP_FILE *inline
zzip_creat(zzip_char_t * name, int o_mode)
{
    return zzip_file_creat(zzip_savefile, name, mode);
}
#endif


#if 0                           /* pure documentation */

/** start writing to the magic zzip_savefile   also: zzip_creat, zzip_write
 * 
 * open a zip archive for writing via the magic zzip_savefile macro
 * variable. The name and mode are given to => zzip_createdir and
 * the result is stored into => zzip_savefile - if the => zzip_savefile
 * did already have a zzip_dir handle then it is automatically 
 * finalized with => zzip_sync and the handle closed and the
 * zzip_savefile variable reused for the new zip archive just started
 * with this call. - This function is really a preprocessor macro 
 * or preferably an inline function around => zzip_dir_create, there 
 * is no such symbol generated into the library.
 #ifndef zzip_savefile
 #define zzip_savefile 0
 #endif
 #define zzip_start(name,mode,ext) \ -
       { if (zzip_savefile) zzip_closedir(zzip_savefile); \ -
          zzip_savefile = zzip_createdir(name,mode,ext); }
 * This function returns null on error or a zzip_dir handle on
 * success. It is perfectly okay to continue with a null in the
 * zzip_savefile variable since it makes subsequent calls to
 * => zzip_creat and => zzip_mkdir to run as => creat(2) / => mkdir(2) 
 * on the real filesystem.
 */
void inline
zzip_mkfifo(zzip_char_t * name, int o_mode)
{
    if (zzip_savefile)
        zzip_closedir(zzip_savefile);
    zzip_savefile = zzip_createdir(name, o_mode);
}
#endif

#if 0                           /* pure documentation */

/** => zzip_mkfifo                        also: zzip_closedir, sync(2)
 * 
 * finalize a zip archive thereby writing the central directory to
 * the end of the file. If it was a real directory then we do just
 * nothing - even that the prototype of the call itself is modelled 
 * to be similar to the posix => sync(2) call. This function is 
 * really a preprocessor macro or preferably an inline function
 * around => zzip_closedir, there is no such symbol generated 
 * into the library.
 #ifndef zzip_savefile
 #define zzip_savefile 0
 #endif
 #define zzip_sync(name,mode) \ -
       { zzip_closedir(zzip_savefile); zzip_savefile = 0; }
 *
 */
void inline
zzip_sync(void)
{
    zzip_closedir(zzip_savefile);
    zzip_savefile = 0;
}
#endif

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
