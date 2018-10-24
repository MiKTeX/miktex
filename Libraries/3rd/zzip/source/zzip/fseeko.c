
/*
 * NOTE: this is part of libzzipfseeko (i.e. it is not libzzip).
 *                                           ==================
 *
 * These routines are fully independent from the traditional zzip
 * implementation. They assume a readonly seekable stdio handle
 * representing a complete zip file. The functions show how to
 * parse the structure, find files and return a decoded bytestream.
 *
 * These routines are a bit simple and really here for documenting
 * the way to access a zip file. The complexity of zip access comes
 * from staggered reading of bytes and reposition of a filepointer in
 * a big archive with lots of files and long compressed datastreams.
 * Plus varaints of drop-in stdio replacements, obfuscation routines,
 * auto fileextensions, drop-in dirent replacements, and so on...
 *
 * btw, we can _not_ use fgetpos/fsetpos since an fpos_t has no asserted
 * relation to a linear seek value as specified in zip info headers. In
 * general it is not a problem if your system has no fseeko/ftello pair
 * since we can fallback to fseek/ftell which limits the zip disk size
 * to 2MiBs but the zip-storable seek values are 32bit limited anyway.
 *
 * Author:
 *      Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) Guido Draheim, use under copyleft (LGPL,MPL)
 */

#define _LARGEFILE_SOURCE 1
#define _ZZIP_ENTRY_STRUCT 1

#include <zzip/fseeko.h>

#include <zzip/fetch.h>
#include <zzip/__mmap.h>
#include <zzip/__fnmatch.h>
#include <zzip/__errno.h>

#include <stdlib.h>
#include <sys/stat.h>

#if   defined ZZIP_HAVE_STRING_H
#include <string.h>
#elif defined ZZIP_HAVE_STRINGS_H
#include <strings.h>
#endif

#if   defined ZZIP_HAVE_STDINT_H
#include <stdint.h>
#endif

#if __STDC_VERSION__+0 > 199900L
#define ___
#define ____
#else
#define ___ {
#define ____ }
#endif

#ifndef ZZIP_HAVE_FSEEKO
#define fseeko fseek
#define ftello ftell
#endif

/* note that the struct zzip_entry inherits the zzip_disk_entry values
 * and usually carries a copy of its values (in disk format!). To make the
 * following code more readable, we use a shorthand notation for the
 * upcast needed in C (not needed in C++) as "disk_(entry)".
 */
#ifdef __zzip_entry_extends_zzip_disk_entry
#define disk_(_entry_) _entry_
#else
#define disk_(_entry_) (& (_entry_)->head)
#endif

/* we try to round all seeks to the pagesize - since we do not use
 * the sys/mmap interface we have to guess a good value here: */
#define PAGESIZE 8192

#ifdef DEBUG
#define debug1(msg) do { fprintf(stderr, "DEBUG: %s : " msg "\n", __func__); } while(0)
#define debug2(msg, arg1) do { fprintf(stderr, "DEBUG: %s : " msg "\n", __func__, arg1); } while(0)
#define debug3(msg, arg1, arg2) do { fprintf(stderr, "DEBUG: %s : " msg "\n", __func__, arg1, arg2); } while(0)
#define debug4(msg, arg1, arg2, arg3) do { fprintf(stderr, "DEBUG: %s : " msg "\n", __func__, arg1, arg2, arg3); } while(0)
#else
#define debug1(msg) 
#define debug2(msg, arg1) 
#define debug3(msg, arg1, arg2) 
#define debug4(msg, arg1, arg2, arg3) 
#endif

/* ====================================================================== */

/*                      helper functions                                  */

/** => zzip_entry_data_offset
 * This functions read the correspoding struct zzip_file_header from
 * the zip disk of the given "entry". The returned off_t points to the
 * end of the file_header where the current fseek pointer has stopped.
 * This is used to immediately parse out any filename/extras block following
 * the file_header. 
 *
 * Returns zero on error. (errno = EINVAL|EBADMSG|EBADF|EIO)
 */
static zzip_off_t
zzip_entry_fread_file_header(ZZIP_ENTRY * entry,
                             struct zzip_file_header *file_header)
{
    if (! entry || ! file_header)
    {
        errno = EINVAL;
        return 0;
    }
    ___ zzip_off_t offset = zzip_disk_entry_fileoffset(disk_(entry));
    if (0 > offset || offset >= entry->disksize)
    {
        errno = EBADMSG;
        return 0;
    }

    if (fseeko(entry->diskfile, offset, SEEK_SET) == -1) 
    {
        debug2("fseeko failed: %s", strerror(errno));
        return 0; /* EBADF */
    }

    ___ zzip_size_t n = fread(file_header, 1, sizeof(*file_header), entry->diskfile);
    if (n < sizeof(*file_header))
    {
        debug4("fread failed: at offset %llu got %i instead of %i", 
            (long long) offset, n, sizeof(*file_header));
        errno = ferror(entry->diskfile) ? EBADF : EIO;
        return 0;
    }
    ____;
    return offset + sizeof(*file_header);
    ____;
}

/** helper functions for (fseeko) zip access api
 *
 * This functions returns the seekval offset of the data portion of the
 * file referenced by the given zzip_entry. It requires an intermediate
 * check of the file_header structure (i.e. it reads it from disk). After
 * this call, the contained diskfile readposition is already set to the
 * data_offset returned here. 
 *
 * Returns -1 on error. (errno = EINVAL|EBADMSG)
 */
zzip_off_t
zzip_entry_data_offset(ZZIP_ENTRY * entry)
{
    struct zzip_file_header file_header;
    if (! entry)
    {
        errno = EINVAL;
        return -1;
    }
    ___ zzip_off_t offset = zzip_entry_fread_file_header(entry, &file_header);
    if (! offset)
    {
        return -1; /* EBADMSG */
    }
    offset += zzip_file_header_sizeof_tails(&file_header);
    if (fseeko(entry->diskfile, offset, SEEK_SET) == -1)
    {
        return -1; /* EBADF */
    }
    return offset;
    ____;
}

/** => zzip_entry_data_offset
 * This function is a big helper despite its little name: in a zip file the
 * encoded filenames are usually NOT zero-terminated but for common usage
 * with libc we need it that way. Secondly, the filename SHOULD be present
 * in the zip central directory but if not then we fallback to the filename
 * given in the file_header of each compressed data portion.
 *
 * returns: new string buffer, null on error (errno = EINVAL|ENOMEM|EBADMSG)
 */
zzip__new__ char *
zzip_entry_strdup_name(ZZIP_ENTRY * entry)
{
    if (! entry)
    {
        errno = EINVAL;
        return 0;
    }

    ___ zzip_size_t len;
    if ((len = zzip_disk_entry_namlen(disk_(entry))))
    {
        char *name = malloc(len + 1);
        if (! name)
        {
            return 0; /* ENOMEM */
        }
        memcpy(name, entry->tail, len);
        name[len] = '\0';
        return name;
    }
    ___ auto struct zzip_file_header header;
    if (zzip_entry_fread_file_header(entry, &header)
        && (len = zzip_file_header_namlen(&header)))
    {
        char *name = malloc(len + 1);
        if (! name) {
            return 0; /* ENOMEM */
        } 
        ___ zzip_size_t n = fread(name, 1, len, entry->diskfile);
        if (n < len) 
        {
            errno = ferror(entry->diskfile) ? EBADF : EIO;
            free (name);
            return 0;
        }
        name[n] = '\0';
        return name;
        ____;
    }
    errno = EBADMSG;
    return 0;
    ____;
    ____;
}

static int
prescan_entry(ZZIP_ENTRY * entry)
{
    if (! entry) 
        return EINVAL;
    ___ zzip_off_t tailsize = zzip_disk_entry_sizeof_tails(disk_(entry));
    if (tailsize + 1 > entry->tailalloc)
    {
        char *newtail = realloc(entry->tail, tailsize + 1);
        if (! newtail)
            return ENOMEM;
        entry->tail = newtail;
        entry->tailalloc = tailsize + 1;
    }
#  ifdef SIZE_MAX /* from stdint.h */
    if ((unsigned long long)(tailsize) > (unsigned long long)(SIZE_MAX)) 
    {
        debug3("tailsize %llu bigger than sizemax %llu", (long long)(tailsize), (long long)(SIZE_MAX));
        return EFBIG; 
    }
#  endif
    ___ zzip_size_t readsize = fread(entry->tail, 1, tailsize, entry->diskfile);
    /* name + comment + extras */
    if ((zzip_off_t)readsize != tailsize) {
        debug1("read to data");
        return errno;
    } else {
        return 0;
    } ____; ____;
}

static void
prescan_clear(ZZIP_ENTRY * entry)
{
    if (! entry)
        return;
    if (entry->tail)
        free(entry->tail);
    entry->tail = 0;
    entry->tailalloc = 0;
}

/* ====================================================================== */

/** => zzip_entry_findfile
 *
 * This function is the first call of all the zip access functions here.
 * It contains the code to find the first entry of the zip central directory.
 * Here we require the stdio handle to represent a real zip file where the
 * disk_trailer is _last_ in the file area, so that its position would be at
 * a fixed offset from the end of the file area if not for the comment field
 * allowed to be of variable length (which needs us to do a little search
 * for the disk_tailer). However, in this simple implementation we disregard
 * any disk_trailer info telling about multidisk archives, so we just return
 * a pointer to the first entry in the zip central directory of that file.
 *
 * For an actual means, we are going to search backwards from the end
 * of the mmaped block looking for the PK-magic signature of a
 * disk_trailer. If we see one then we check the rootseek value to
 * find the first disk_entry of the root central directory. If we find
 * the correct PK-magic signature of a disk_entry over there then we
 * assume we are done and we are going to return a pointer to that label.
 *
 * The return value is a pointer to the first zzip_disk_entry being checked
 * to be within the bounds of the file area specified by the arguments. If
 * no disk_trailer was found then null is returned, and likewise we only
 * accept a disk_trailer with a seekvalue that points to a disk_entry and
 * both parts have valid PK-magic parts. Beyond some sanity check we try to
 * catch a common brokeness with zip archives that still allows us to find
 * the start of the zip central directory.
 *
 * Returns null on error (errno = EINVAL|ENOMEM|EBADMSG|EBADF|ENOENT)
 */
zzip__new__ ZZIP_ENTRY *
zzip_entry_findfirst(FILE * disk)
{
    if (! disk)
    {
        errno = EINVAL;
        return 0;
    }
    if (fseeko(disk, 0, SEEK_END) == -1)
    {
        printf("fseeko failed: %s", strerror(errno));
        return 0; /* EBADF */
    }
    ___ zzip_off_t disksize = ftello(disk);
    if (disksize < (zzip_off_t) sizeof(struct zzip_disk_trailer))
    {
        errno = EBADMSG;
        return 0;
    }
    /* we read out chunks of 8 KiB in the hope to match disk granularity */
    ___ zzip_off_t pagesize = PAGESIZE; /* getpagesize() */
    ___ ZZIP_ENTRY *entry = calloc(1, sizeof(*entry));
    if (! entry)
        goto error0; /* ENOMEM */
    ___ unsigned char *buffer = malloc(pagesize);
    if (! buffer)
        goto error1; /* ENOMEM */

    if (pagesize / 2 <= (zzip_off_t) sizeof(struct zzip_disk_trailer))
    {
        errno = EBADMSG;
        goto error2;
    }
    /* at each step, we will fread a pagesize block which overlaps with the
     * previous read by means of pagesize/2 step at the end of the while(1) */
    ___ zzip_off_t mapoffs = disksize & ~(pagesize - 1);
    ___ zzip_off_t mapsize = disksize - mapoffs;
    if (mapoffs && mapsize < pagesize / 2)
    {
        mapoffs -= pagesize / 2;
        mapsize += pagesize / 2;
    }
    if (mapsize >= 3*8192)
    {
        errno = EBADMSG;
        goto error2;
    }
    while (1)
    {
        if (fseeko(disk, mapoffs, SEEK_SET) == -1)
            goto error2; /* EBADF */
        if (fread(buffer, 1, mapsize, disk) < (zzip_size_t)mapsize)
        {
            errno = ferror(disk) ? EBADF : EIO;
            debug1("in fread");
            goto error2;
        }
        ___ unsigned char *p =
            buffer + mapsize - sizeof(struct zzip_disk_trailer);
        for (; p >= buffer; p--)
        {
            zzip_off_t root;    /* (struct zzip_disk_entry*) */
            if (zzip_disk_trailer_check_magic(p))
            {
                root = zzip_disk_trailer_rootseek((struct zzip_disk_trailer *)
                                                  p);
                if (root > disksize - (long) sizeof(struct zzip_disk_trailer))
                {
                    /* first disk_entry is after the disk_trailer? can't be! */
                    struct zzip_disk_trailer *trailer =
                        (struct zzip_disk_trailer *) p;
                    zzip_off_t rootsize = zzip_disk_trailer_rootsize(trailer);
                    if (rootsize > mapoffs)
                        continue;
                    /* a common brokeness that can be fixed: we just assume the
                     * central directory was written directly before : */
                    root = mapoffs - rootsize;
                }
            } else if (zzip_disk64_trailer_check_magic(p))
            {
                struct zzip_disk64_trailer *trailer =
                    (struct zzip_disk64_trailer *) p;
                if (sizeof(zzip_off_t) < 8)
                {
                    debug1("disk64 trailer on non-large compile");
                    errno = EFBIG;
                    goto error2;
                }
                if ((void*)(trailer + 1) > (buffer + mapsize))
                {
                    debug1("disk64 trailer is not complete");
                    errno = EBADMSG;
                    goto error2; 
                }
                root = zzip_disk64_trailer_rootseek(trailer);
            } else
                continue;

            if (!(0 <= root && root < disksize))
            {
                debug3("bogus rootseek value %lli (disksize %lli)", (long long)root, (long long)disksize);
                errno = EBADMSG;
                goto error2;
            }
            if (fseeko(disk, root, SEEK_SET) == -1)
            {
                debug2("next seeko %s", strerror(errno));
                goto error2; /* EBADF */
            }
            if (fread(disk_(entry), 1, sizeof(*disk_(entry)), disk)
                    != sizeof(*disk_(entry))) 
            {   
                debug2("next freed %s", strerror(errno));
                errno = ferror(disk) ? EBADF : EIO;
                goto error2;
            }
            if (zzip_disk_entry_check_magic(entry))
            {
                free(buffer);
                entry->headseek = root;
                entry->diskfile = disk;
                entry->disksize = disksize;
                ___ int err = prescan_entry(entry);
                if (err) 
                {
                    debug2("prescan %s", strerror(err));
                    errno = err;
                    goto error2;
                }
                return entry;
                ____;
            }
        }
        ____;
        if (! mapoffs)
            break;
        if (mapsize < pagesize / 2)
        {
            debug1("bad mapsize should not be possible");
            errno = EBADMSG;
            goto error2;
        }
        mapoffs -= pagesize / 2;        /* mapsize += pagesize/2; */
        mapsize = pagesize;     /* if (mapsize > pagesize) ... */
        if (disksize - mapoffs > 64 * 1024)
            break;
    }
    errno = ENOENT; /* not found */
  error2:
    free(buffer);
  error1:
    free(entry);
    ____;
    ____;
    ____;
    ____;
    ____;
    ____;
  error0:
    debug1("error..");
    return 0;
}

/** => zzip_entry_findfile
 *
 * This function takes an existing "entry" in the central root directory
 * (e.g. from zzip_entry_findfirst) and moves it to point to the next entry.
 * On error it returns 0, otherwise the old entry. If no further match is
 * found then null is returned and the entry already free()d. If you want
 * to stop searching for matches before that case then please call
 * => zzip_entry_free on the cursor struct ZZIP_ENTRY.
 */
zzip__new__ ZZIP_ENTRY *
zzip_entry_findnext(ZZIP_ENTRY * _zzip_restrict entry)
{
    if (! entry)
    {
        errno = EINVAL;
        return entry;
    }
    if (! zzip_disk_entry_check_magic(entry))
        goto error_EBADMSG;
    ___ zzip_off_t seek =
        entry->headseek + zzip_disk_entry_sizeto_end(disk_(entry));
    if (seek + (zzip_off_t) sizeof(*disk_(entry)) > entry->disksize)
        goto error_EBADMSG;

    if (fseeko(entry->diskfile, seek, SEEK_SET) == -1)
        goto error; /* EBADF */
    if (fread(disk_(entry), 1, sizeof(*disk_(entry)), entry->diskfile)
            < sizeof(*disk_(entry))) 
    {
        errno = ferror(entry->diskfile) ? EBADF : EIO;
        goto error;
    }
    entry->headseek = seek;
    if (! zzip_disk_entry_check_magic(entry))
        goto error_EBADMSG;
    ___ int err = prescan_entry(entry);
    if (err)
    {
        errno = err;
        goto error;
    }
    return entry;
    ____;
  error_EBADMSG:
    errno = EBADMSG;
  error:
    zzip_entry_free(entry);
    return 0;
    ____;
}

/** => zzip_entry_findfile
 * this function releases the malloc()ed areas needed for zzip_entry, the
 * pointer is invalid afterwards. This function has #define synonyms of
 * zzip_entry_findlast(), zzip_entry_findlastfile(), zzip_entry_findlastmatch()
 */
int
zzip_entry_free(ZZIP_ENTRY * entry)
{
    if (! entry)
    {
        errno = EINVAL;
        return 0;
    }
    prescan_clear(entry);
    free(entry);
    return 1;
}

/** search for files in the (fseeko) zip central directory
 *
 * This function is given a filename as an additional argument, to find the
 * disk_entry matching a given filename. The compare-function is usually
 * strcmp or strcasecmp or perhaps strcoll, if null then strcmp is used.
 * - use null as argument for "old"-entry when searching the first
 * matching entry, otherwise the last returned value if you look for other
 * entries with a special "compare" function (if null then a doubled search
 * is rather useless with this variant of _findfile). If no further entry is
 * found then null is returned and any "old"-entry gets already free()d.
 */
zzip__new__ ZZIP_ENTRY *
zzip_entry_findfile(FILE * disk, char *filename,
                    ZZIP_ENTRY * _zzip_restrict entry, zzip_strcmp_fn_t compare)
{
    if (! filename || ! disk)
    {
        errno = EINVAL;
        return 0;
    }
    if (! entry)
        entry = zzip_entry_findfirst(disk);
    else
        entry = zzip_entry_findnext(entry);

    if (! compare)
        compare = (zzip_strcmp_fn_t) (strcmp);

    for (; entry; entry = zzip_entry_findnext(entry))
    {
        /* filenames within zip files are often not null-terminated! */
        char *realname = zzip_entry_strdup_name(entry);
        if (! realname)
        {
            return 0; /* ENOMEM|EBADMSG */
        }
        if (! compare(filename, realname))
        {
            free(realname);
            return entry;
        } else
        {
            free(realname);
            continue;
        }
    }
    errno = ENOENT;
    return 0;
}

/** => zzip_entry_findfile
 *
 * This function uses a compare-function with an additional argument
 * and it is called just like fnmatch(3) from POSIX.2 AD:1993), i.e.
 * the argument filespec first and the ziplocal filename second with
 * the integer-flags put in as third to the indirect call. If the
 * platform has fnmatch available then null-compare will use that one
 * and otherwise we fall back to mere strcmp, so if you need fnmatch
 * searching then please provide an implementation somewhere else.
 * - use null as argument for "after"-entry when searching the first
 * matching entry, or the last disk_entry return-value to find the
 * next entry matching the given filespec. If no further entry is
 * found then null is returned and any "old"-entry gets already free()d.
 */
zzip__new__ ZZIP_ENTRY *
zzip_entry_findmatch(FILE * disk, char *filespec,
                     ZZIP_ENTRY * _zzip_restrict entry,
                     zzip_fnmatch_fn_t compare, int flags)
{
    if (! filespec || ! disk)
    {
        errno = EINVAL;
        return 0;
    }
    if (! entry)
        entry = zzip_entry_findfirst(disk);
    else
        entry = zzip_entry_findnext(entry);

    if (! compare)
        compare = (zzip_fnmatch_fn_t) _zzip_fnmatch;

    for (; entry; entry = zzip_entry_findnext(entry))
    {
        /* filenames within zip files are often not null-terminated! */
        char *realname = zzip_entry_strdup_name(entry);
        if (! realname)
        {
            return 0; /* ENOMEM|EBADMSG */
        }
        if (! compare(filespec, realname, flags))
        {
            debug3("compare '%s' equal with zip '%s'", filespec, realname);
            free(realname);
            return entry;
        } else
        {
            debug3("compare '%s' not like zip '%s'", filespec, realname);
            free(realname);
            continue;
        }
    }
    errno = ENOENT;
    return 0;
}

/* ====================================================================== */

/**
 * typedef struct zzip_disk_file ZZIP_ENTRY_FILE;
 */
struct zzip_entry_file          /* : zzip_file_header */
{
    struct zzip_file_header header;     /* fopen detected header */
    ZZIP_ENTRY *entry;          /* fopen entry */
    zzip_off_t data;            /* for stored blocks */
    zzip_size_t avail;          /* memorized for checks on EOF */
    zzip_size_t compressed;     /* compressed flag and datasize */
    zzip_size_t dataoff;        /* offset from data start */
    z_stream zlib;              /* for inflated blocks */
    unsigned char buffer[PAGESIZE];     /* work buffer for inflate algorithm */
};

/** open a file within a zip disk for reading
 *
 * This function does take an "entry" argument and copies it (or just takes
 * it over as owner) to a new ZZIP_ENTRY_FILE handle structure. That
 * structure contains also a zlib buffer for decoding. This function does
 * seek to the file_header of the given "entry" and validates it for the
 * data buffer following it. We do also prefetch some data from the data
 * buffer thereby trying to match the disk pagesize for faster access later.
 * The => zzip_entry_fread will then read in chunks of pagesizes which is
 * the size of the internal readahead buffer. If an error occurs then null
 * is returned.
 */
zzip__new__ ZZIP_ENTRY_FILE *
zzip_entry_fopen(ZZIP_ENTRY * entry, int takeover)
{
    if (! entry)
    {
        errno = EINVAL;
        return 0;
    }
    if (! takeover)
    {
        ZZIP_ENTRY *found = malloc(sizeof(*entry));
        if (! found)
        {
            return 0; /* ENOMEM */
        }
        memcpy(found, entry, sizeof(*entry));   /* prescan_copy */
        found->tail = malloc(found->tailalloc);
        if (! found->tail) 
        {
            free (found); 
            return 0; /* ENOMEM */
        }
        memcpy(found->tail, entry->tail, entry->tailalloc);
        entry = found;
    }
    ___ ZZIP_ENTRY_FILE *file = malloc(sizeof(*file));
    if (! file)
        goto error1; /* ENOMEM */
    file->entry = entry;
    if (! zzip_entry_fread_file_header(entry, &file->header))
    {
        debug2("file header: %s", strerror(errno));
        goto error2; /* EIO|EBADMSG */
    }
    file->avail = zzip_file_header_usize(&file->header);
    file->data = zzip_entry_data_offset(entry);
    file->dataoff = 0;

    if (! file->avail || zzip_file_header_data_stored(&file->header))
        { file->compressed = 0; return file; }

    file->compressed = zzip_file_header_csize(&file->header);
    file->zlib.opaque = 0;
    file->zlib.zalloc = Z_NULL;
    file->zlib.zfree = Z_NULL;

    ___ zzip_off_t size = file->avail;
    if (size > sizeof(file->buffer))
        size = sizeof(file->buffer);
    if (fseeko(file->entry->diskfile, file->data + file->dataoff, SEEK_SET) == -1)
    {
        debug2("file seek failed: %s", strerror(errno));
        goto error2; /* EBADF */
    }
    file->zlib.next_in = file->buffer;
    file->zlib.avail_in = fread(file->buffer, 1, size,
                                file->entry->diskfile);
    file->dataoff += file->zlib.avail_in;
    ____;

    if (! zzip_file_header_data_deflated(&file->header)
        || inflateInit2(&file->zlib, -MAX_WBITS) != Z_OK)
    {
        debug1("decompress failed");
        errno = EBADMSG;
        goto error2;
    }

    return file;
  error2:
    free(file);
  error1:
    zzip_entry_free(entry);
    return 0;
    ____;
}

/** => zzip_entry_fopen
 *
 * This function opens a file found by name, so it does a search into
 * the zip central directory with => zzip_entry_findfile and whatever
 * is found first is given to => zzip_entry_fopen
 */
zzip__new__ ZZIP_ENTRY_FILE *
zzip_entry_ffile(FILE * disk, char *filename)
{
    ZZIP_ENTRY *entry = zzip_entry_findfile(disk, filename, 0, 0);
    if (! entry)
    {
        return 0; /* EBADMSG */
    }
    return zzip_entry_fopen(entry, 1);
}


/** => zzip_entry_fopen
 *
 * This function reads more bytes into the output buffer specified as
 * arguments. The return value is null on eof or error, the stdio-like
 * interface can not distinguish between these so you need to check
 * with => zzip_entry_feof for the difference.
 */
zzip_size_t
zzip_entry_fread(void *ptr, zzip_size_t sized, zzip_size_t nmemb,
                 ZZIP_ENTRY_FILE * file)
{
    if (! file)
    {
        errno = EINVAL;
        return 0;
    }
    ___ zzip_size_t size = sized * nmemb;
    if (! file->compressed)
    {
        if (size > file->avail)
            size = file->avail;
        if (fread(ptr, 1, size, file->entry->diskfile) != size) 
        {
            errno = ferror(file->entry->diskfile) ? EBADF : EIO;
            debug2("uncompressed fread incomplete: %s", strerror(errno));
            return 0;
        }
        file->dataoff += size;
        file->avail -= size;
        return size;
    }

    file->zlib.avail_out = size;
    file->zlib.next_out = ptr;
    ___ zzip_size_t total_old = file->zlib.total_out;
    while (1)
    {
        if (! file->zlib.avail_in)
        {
            size = file->compressed - file->dataoff;
            debug2("remaining compressed bytes: %lli", (long long)size);
            if (size > sizeof(file->buffer))
                size = sizeof(file->buffer);
            /* fseek (file->data + file->dataoff, file->entry->diskfile); */
            file->zlib.avail_in = fread(file->buffer, 1, size,
                                        file->entry->diskfile);
            file->zlib.next_in = file->buffer;
            file->dataoff += file->zlib.avail_in;
            debug2("remaining compressed fread %lli", (long long) file->zlib.avail_in);
        }
        if (! file->zlib.avail_in)
        {
            errno = EIO;
            return 0;
        }

        ___ int err = inflate(&file->zlib, Z_NO_FLUSH);
        if (err == Z_STREAM_END)
            file->avail = 0;
        else if (err == Z_OK)
            file->avail -= file->zlib.total_out - total_old;
        else
        {
            debug1("zlib decode incomplete");
            errno = EBADMSG;
            return 0;
        }
        ____;
        if (file->zlib.avail_out && ! file->zlib.avail_in)
            continue;
        return file->zlib.total_out - total_old;
    }
    ____;
    ____;
}

/** => zzip_entry_fopen
 * This function releases any zlib decoder info needed for decompression
 * and dumps the ZZIP_ENTRY_FILE struct then.
 */
int
zzip_entry_fclose(ZZIP_ENTRY_FILE * file)
{
    if (! file)
    {
        errno = EINVAL;
        return EOF;
    }
    if (file->compressed)
        inflateEnd(&file->zlib);
    zzip_entry_free(file->entry);
    free(file);
    return 0;
}

/** => zzip_entry_fopen
 *
 * This function allows to distinguish an error from an eof condition.
 * Actually, if we found an error but we did already reach eof then we
 * just keep on saying that it was an eof, so the app can just continue.
 */
int
zzip_entry_feof(ZZIP_ENTRY_FILE * file)
{
    return ! file || ! file->avail;
}
