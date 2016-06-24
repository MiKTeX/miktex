#ifndef __ZZIP_MEMDISK_H
#define __ZZIP_MEMDISK_H

/* NOTE: this is part of libzzipmmapped (i.e. it is not libzzip). */

#include <zzip/types.h>
#include <zzip/mmapped.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _zzip_mem_disk ZZIP_MEM_DISK;
typedef struct _zzip_mem_entry ZZIP_MEM_ENTRY;

struct _zzip_mem_disk {
    ZZIP_DISK* disk;
    ZZIP_MEM_ENTRY* list;
    ZZIP_MEM_ENTRY* last;
};

#ifndef zzip_mem_disk_extern
#define zzip_mem_disk_extern
#endif

zzip_mem_disk_extern zzip__new__ ZZIP_MEM_DISK*
zzip_mem_disk_new (void);
zzip_mem_disk_extern zzip__new__ ZZIP_MEM_DISK*
zzip_mem_disk_open (char* filename);
zzip_mem_disk_extern zzip__new__ ZZIP_MEM_DISK*
zzip_mem_disk_fdopen (int fd);
zzip_mem_disk_extern zzip__new__ ZZIP_MEM_DISK *
zzip_mem_disk_buffer(char *buffer, size_t buflen);
zzip_mem_disk_extern void
zzip_mem_disk_close (ZZIP_MEM_DISK* _zzip_restrict dir);

zzip_mem_disk_extern long
zzip_mem_disk_load (ZZIP_MEM_DISK* dir, ZZIP_DISK* disk);
zzip_mem_disk_extern void
zzip_mem_disk_unload (ZZIP_MEM_DISK* dir);
ZZIP_EXTRA_BLOCK*
zzip_mem_entry_extra_block (ZZIP_MEM_ENTRY* entry, short datatype);

#ifdef USE_INLINE
_zzip_inline ZZIP_DISK* zzip_disk (ZZIP_MEM_DISK* dir) { return dir->disk; }
#else
#define zzip_disk(_X_) ((_X_)->disk)
#endif


/* these functions are much faster than the orgiinal zzip_disk_ functions */

/* zzip_mem_entry <is similar to> zzip_disk_entry */
struct _zzip_mem_entry {
    struct _zzip_mem_entry* zz_next;
    char*            zz_name;      /* zero-terminated (from "filename") */
    zzip_byte_t*     zz_data;      /* compressed content start (mmap addr) */
    int              zz_flags;     /* (from "z_flags") */
    int              zz_compr;     /* (from "z_compr") */
    long             zz_mktime;    /* (from "z_dostime") */
    long             zz_crc32;     /* (from "z_crc32") */
    zzip_off_t       zz_csize;     /* (from "z_csize")  overridden by zip64 */
    zzip_off_t       zz_usize;     /* (from "z_usize")  overridden by zip64 */
    zzip_off_t       zz_offset;    /* (from "z_offset") overridden by zip64 */
    int              zz_diskstart; /* (from "z_diskstart") rridden by zip64 */
    int              zz_filetype;  /* (from "z_filetype") */
    char*            zz_comment;   /* zero-terminated (from "comment") */
    ZZIP_EXTRA_BLOCK* zz_ext[3];   /* terminated by null in z_datatype */
};                                 /* the extra blocks are NOT converted */

#define _zzip_mem_disk_findfirst(_d_) ((_d_)->list)
#define _zzip_mem_disk_findnext(_d_,_e_) (!(_e_)?(_d_)->list:(_e_)->zz_next)
#define _zzip_mem_entry_findnext(_e_) ((_e_)->zz_next)

#ifndef USE_INLINE
#define zzip_mem_disk_findfirst _zzip_mem_disk_findfirst
#define zzip_mem_disk_findnext _zzip_mem_disk_findnext
#define zzip_mem_entry_findnext _zzip_mem_entry_findnext
#else

_zzip_inline ZZIP_MEM_ENTRY*
zzip_mem_disk_findfirst(ZZIP_MEM_DISK* dir) {
    if (! dir) return 0;
    return _zzip_mem_disk_findfirst(dir); }
_zzip_inline ZZIP_MEM_ENTRY*
zzip_mem_disk_findnext(ZZIP_MEM_DISK* dir, ZZIP_MEM_ENTRY* entry) {
    if (! dir) return 0;
    return _zzip_mem_disk_findnext(dir, entry); }
_zzip_inline ZZIP_MEM_ENTRY*
zzip_mem_entry_findnext(ZZIP_MEM_ENTRY* entry) {
    if (! entry) return 0;
    return _zzip_mem_entry_findnext(entry); }
#endif

#define _zzip_mem_entry_to_name(_e_) ((_e_)->zz_name)
#define _zzip_mem_entry_to_comment(_e_) ((_e_)->zz_comment)
#define _zzip_mem_entry_strdup_name(_e_) (strdup((_e_)->zz_name))
#define _zzip_mem_entry_to_data(_e_) ((_e_)->zz_data)

#ifndef USE_INLINE
#define zzip_mem_entry_to_name _zzip_mem_entry_to_name
#define zzip_mem_entry_to_comment _zzip_mem_entry_to_comment
#define zzip_mem_entry_strdup_name _zzip_mem_entry_strdup_name
#define zzip_mem_entry_to_data _zzip_mem_entry_to_data
#else
_zzip_inline char*
zzip_mem_entry_to_name(ZZIP_MEM_ENTRY* entry) {
    if (! entry) return 0;
    return _zzip_mem_entry_to_name(entry); }
_zzip_inline char*
zzip_mem_entry_to_comment(ZZIP_MEM_ENTRY* entry) {
    if (! entry) return 0;
    return _zzip_mem_entry_to_comment(entry); }
zzip__new__
_zzip_inline char*
zzip_mem_entry_strdup_name(ZZIP_MEM_ENTRY* entry) {
    if (! entry) return 0;
    return _zzip_mem_entry_strdup_name(entry); }
_zzip_inline zzip_byte_t*
zzip_mem_entry_to_data(ZZIP_MEM_ENTRY* entry) {
    if (! entry) return 0;
    return _zzip_mem_entry_to_data(entry); }
#endif

ZZIP_MEM_ENTRY*
zzip_mem_disk_findfile(ZZIP_MEM_DISK* dir,
                       char* filename, ZZIP_MEM_ENTRY* after,
		       zzip_strcmp_fn_t compare);

ZZIP_MEM_ENTRY*
zzip_mem_disk_findmatch(ZZIP_MEM_DISK* dir,
                        char* filespec, ZZIP_MEM_ENTRY* after,
			zzip_fnmatch_fn_t compare, int flags);

/* named access -------------------------------------------------------- */
#define zzip_mem_entry_usize(_e_) ((_e_)->zz_usize)
#define zzip_mem_entry_csize(_e_) ((_e_)->zz_csize)
#define zzip_mem_entry_data_encrypted(_e_) ZZIP_IS_ENCRYPTED((_e_)->zz_flags)
#define zzip_mem_entry_data_streamed(_e_) ZZIP_IS_STREAMED((_e_)->zz_flags)
#define zzip_mem_entry_data_comprlevel(_e_) ((_e_)->zz_compr)
#define zzip_mem_entry_data_stored(_e_) ((_e_)->zz_compr == ZZIP_IS_STORED)
#define zzip_mem_entry_data_deflated(_e_) ((_e_)->zz_compr == ZZIP_IS_DEFLATED)

/* zzip_mem_disk_file -------------------------------------------------- */

/* since only the header data is being cached, all the real data
 * operations are actually the same as in mmapped.h - just fopen makes
 * access to the header data in memory instead of the zip archive.
 */

typedef ZZIP_DISK_FILE ZZIP_MEM_DISK_FILE;

zzip__new__ ZZIP_MEM_DISK_FILE*
zzip_mem_entry_fopen (ZZIP_MEM_DISK* dir, ZZIP_MEM_ENTRY* entry);
zzip__new__ ZZIP_MEM_DISK_FILE*
zzip_mem_disk_fopen (ZZIP_MEM_DISK* dir, char* filename);
_zzip_size_t
zzip_mem_disk_fread (void* ptr, _zzip_size_t size, _zzip_size_t nmemb,
                     ZZIP_MEM_DISK_FILE* file);
int
zzip_mem_disk_fclose (ZZIP_MEM_DISK_FILE* file);
int
zzip_mem_disk_feof (ZZIP_MEM_DISK_FILE* file);

/* convert dostime of entry to unix time_t */
long zzip_disk_entry_get_mktime(ZZIP_DISK_ENTRY* entry);

#ifdef __cplusplus
}
#endif
#endif
