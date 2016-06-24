#ifndef _ZZIP_WRITE_H
#define _ZZIP_WRITE_H

#include <zzip/lib.h>

/* ........................................................................
 * write support is not yet implemented
 * zzip/write.c
 */
#define ZZIP_NO_CREAT 1

ZZIP_DIR*    zzip_dir_creat_ext_io(zzip_char_t* name, int o_mode, 
                                   zzip_strings_t* ext, zzip_plugin_io_t io);
ZZIP_DIR*    zzip_dir_creat(zzip_char_t* name, int o_mode);
int          zzip_file_mkdir(ZZIP_DIR* dir, zzip_char_t* name, int o_mode);
ZZIP_FILE*   zzip_file_creat(ZZIP_DIR* dir, zzip_char_t* name, int o_mode);
zzip_ssize_t zzip_file_write(ZZIP_FILE* file, 
                             const void* ptr, zzip_size_t len);

ZZIP_DIR*    zzip_createdir(zzip_char_t* name, int o_mode);
zzip_ssize_t zzip_write(ZZIP_FILE* file, const void* ptr, zzip_size_t len);
zzip_size_t  zzip_fwrite(const void* ptr, zzip_size_t len, 
                         zzip_size_t multiply, ZZIP_FILE* file);
#ifndef zzip_savefile
#define zzip_savefile 0
#define zzip_savefile_is_null
#endif

#ifdef _ZZIP_NO_INLINE
#define zzip_mkdir(_name_,_mode_) \
        zzip_file_mkdir((zzip_savefile),(_name_),(_mode_))
#define zzip_creat(_name_,_mode_) \
        zzip_file_creat((zzip_savefile),(_name_),(_mode_))
#define zzip_sync() \
      { zzip_closedir((zzip_savefile)); (zzip_savefile) = 0; }
#define zzip_start(_name_,_mode_,_ext_) \
      { if ((zzip_savefile)) zzip_closedir((zzip_savefile)); 
         zzip_savefile = zzip_dir_creat(_name_, _mode_,_ext_); }

#else

_zzip_inline static int         zzip_mkdir(zzip_char_t* name, int o_mode)
{                   return zzip_file_mkdir(zzip_savefile, name, o_mode); }
_zzip_inline static ZZIP_FILE*  zzip_creat(zzip_char_t* name, int o_mode)
{                   return zzip_file_creat(zzip_savefile, name, o_mode); }

#ifndef zzip_savefile_is_null
_zzip_inline static void        zzip_sync(void)
{                           zzip_closedir(zzip_savefile); zzip_savefile = 0; }
_zzip_inline static void        zzip_mkfifo(zzip_char_t* name, int o_mode)
{       if ((zzip_savefile)) zzip_closedir (zzip_savefile);
             zzip_savefile = zzip_createdir(_name_,_mode_); }
#else
_zzip_inline static void        zzip_sync(void) {}
_zzip_inline static void        zzip_mkfifo(zzip_char_t* name, int o_mode) {}
#endif
#endif /* _ZZIP_NO_INLINE */
#endif /* _ZZIP_WRITE_H */
