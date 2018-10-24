#ifndef _ZZIP_FSEEKO_H_
#define _ZZIP_FSEEKO_H_
/*
 * NOTE: this is part of libzzipfseeko (i.e. it is not libzzip). 
 *
 * simplified zip disk access using fseeko on a stdio FILE handle
 *
 * Author: 
 *      Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) Guido Draheim, use under copyleft (LGPL,MPL)
 */


#include <zzip/types.h>
#include <stdio.h>

#ifdef _ZZIP_ENTRY_STRUCT
#include <zlib.h>
#include <zzip/format.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zzip_entry ZZIP_ENTRY;
typedef struct zzip_entry_file ZZIP_ENTRY_FILE;

typedef int (*zzip_strcmp_fn_t)(char*, char*);
typedef int (*zzip_fnmatch_fn_t)(char*, char*, int);

#ifndef zzip_entry_extern
#define zzip_entry_extern extern
#endif

zzip_entry_extern zzip_off_t
zzip_entry_data_offset(ZZIP_ENTRY* entry);

zzip__new__
zzip_entry_extern ZZIP_ENTRY*
zzip_entry_findfirst(FILE* file);

zzip__new__
zzip_entry_extern ZZIP_ENTRY*
zzip_entry_findnext(ZZIP_ENTRY* _zzip_restrict entry);

#define zzip_entry_findlast      zzip_entry_free
#define zzip_entry_findlastfile  zzip_entry_free
#define zzip_entry_findlastmatch zzip_entry_free

zzip_entry_extern int
zzip_entry_free(ZZIP_ENTRY* entry);

zzip__new__
zzip_entry_extern char*
zzip_entry_strdup_name(ZZIP_ENTRY* entry);
zzip_entry_extern char*
zzip_entry_to_data(ZZIP_ENTRY* entry);

zzip__new__
zzip_entry_extern ZZIP_ENTRY*
zzip_entry_findfile(FILE* disk, char* filename, 
		    ZZIP_ENTRY* _zzip_restrict old,
		   zzip_strcmp_fn_t compare);
zzip__new__
zzip_entry_extern ZZIP_ENTRY*
zzip_entry_findmatch(FILE* disk, char* filespec, 
		     ZZIP_ENTRY* _zzip_restrict old,
		    zzip_fnmatch_fn_t compare, int flags);
zzip__new__
zzip_entry_extern ZZIP_ENTRY_FILE*
zzip_entry_fopen (ZZIP_ENTRY* entry, int takeover);

zzip__new__
zzip_entry_extern ZZIP_ENTRY_FILE*
zzip_entry_ffile (FILE* disk, char* filename);

zzip_entry_extern _zzip_size_t
zzip_entry_fread (void* ptr, _zzip_size_t size, _zzip_size_t nmemb,
		 ZZIP_ENTRY_FILE* file);
zzip_entry_extern int
zzip_entry_fclose (ZZIP_ENTRY_FILE* file);
int
zzip_entry_feof (ZZIP_ENTRY_FILE* file);

# ifdef _ZZIP_ENTRY_STRUCT
#  ifdef __cplusplus
#  define __zzip_entry_extends_zzip_disk_entry
struct zzip_entry : public struct zzip_disk_entry
{
    char*             _zzip_restrict tail;
    zzip_off_t                  tailalloc;   /* the allocated size of tail */
    FILE*                        diskfile;   /* a file reference */
    zzip_off_t                   disksize;   /* the size of the file */
    zzip_off_t                   headseek;   /* the offset within the file */
    zzip_off_t                   zz_usize;
    zzip_off_t                   zz_csize;   /* items scanned from header */
    zzip_off_t                   zz_offset;  /* or zip64 extension block */
    int                          zz_diskstart;
};
# define zzip_entry_d(_p) (_p)
#  else
struct zzip_entry /* : struct zzip_disk_entry */
{
    struct zzip_disk_entry           head;
    char*             _zzip_restrict tail;
    zzip_off_t                  tailalloc;   /* the allocated size of tail */
    FILE*                        diskfile;   /* a file reference */
    zzip_off_t                   disksize;   /* the size of the file */
    zzip_off_t                   headseek;   /* the offset within the file */
    zzip_off_t                   zz_usize;
    zzip_off_t                   zz_csize;   /* items scanned from header */
    zzip_off_t                   zz_offset;  /* or zip64 extension block */
    int                          zz_diskstart;
};
# define zzip_entry_d(_p) (&(_p)->head)
#  endif /* __cplusplus */
# endif /* _ZZIP_MEM_ENTRY_PRIVATE */

#ifdef _ZZIP_ENTRY_STRUCT
/* zzip_entry is-a zip_disk_entry => fetch.h */
#define zzip_entry_csize(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_csize(zzip_entry_d(__p)))
#define zzip_entry_usize(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_usize(zzip_entry_d(__p))) 
#define zzip_entry_compr(__p) ( \
        zzip_disk_entry_get_compr(zzip_entry_d(__p)))
#define zzip_entry_flags(__p) ( \
        zzip_disk_entry_get_flags(zzip_entry_d(__p)))
#define zzip_entry_namlen(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_namlen(zzip_entry_d(__p)))
#define zzip_entry_extras(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_extras(zzip_entry_d(__p)))
#define zzip_entry_comment(__p)   ((zzip_size_t) \
        zzip_disk_entry_get_comment(zzip_entry_d(__p)))
#define zzip_entry_diskstart(__p) ((int) \
        zzip_disk_entry_get_diskstart(zzip_entry_d(__p)))
#define zzip_entry_filetype(__p) ((int) \
        zzip_disk_entry_get_filetype(zzip_entry_d(__p)))
#define zzip_entry_filemode(__p) ((int) \
        zzip_disk_entry_get_filemode(zzip_entry_d(__p)))
#define zzip_entry_fileoffset(__p) ((zzip_off_t) \
        zzip_disk_entry_get_offset(zzip_entry_d(__p)))
#define zzip_entry_sizeof_tail(__p) ((zzip_size_t) \
        zzip_disk_entry_sizeof_tails(zzip_entry_d(__p)))
#endif

#ifdef __cplusplus
extern "C" {
}
#endif
#endif

