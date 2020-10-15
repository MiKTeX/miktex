
/*
 * NOTE: this is part of libzzipmmapped (i.e. it is not libzzip).
 *                                            ==================
 *
 * These routines are fully independent from the traditional zzip
 * implementation. They assume a readonly mmapped sharedmem block
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
 * Author:
 *      Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c)Guido Draheim, use under copyleft (LGPL,MPL)
 */

#define _ZZIP_DISK_FILE_STRUCT 1

#ifdef __linux__
#define _GNU_SOURCE _glibc_developers_are_idiots_to_call_strndup_gnu_specific_
#endif

#include <zzip/mmapped.h>
#include <zzip/format.h>
#include <zzip/fetch.h>
#include <zzip/__debug.h>
#include <zzip/__mmap.h>
#include <zzip/__string.h>
#include <zzip/__fnmatch.h>
#include <zzip/__errno.h>

#include <stdlib.h>
#include <sys/stat.h>

#if   defined ZZIP_HAVE_UNISTD_H
#include <unistd.h>
#elif defined ZZIP_HAVE_IO_H
#include <io.h>
#endif

#if   defined ZZIP_HAVE_STRING_H
#include <string.h>
#elif defined ZZIP_HAVE_STRINGS_H
#include <strings.h>
#endif


#if __STDC_VERSION__+0 > 199900L
#define ___
#define ____
#else
#define ___ {
#define ____ }
#endif

/** => zzip_disk_mmap
 * This function does primary initialization of a disk-buffer struct.
 * 
 * This function always returns 0 as success.
 */
int
zzip_disk_init(ZZIP_DISK * disk, void *buffer, zzip_size_t buflen)
{
    disk->buffer = (zzip_byte_t *) buffer;
    disk->endbuf = (zzip_byte_t *) buffer + buflen;
    disk->reserved = 0;
    disk->flags = 0;
    disk->mapped = 0;
    /* do not touch disk->user */
    /* do not touch disk->code */
    return 0;
}

/** => zzip_disk_mmap
 * This function allocates a new disk-buffer with => malloc(3)
 *
 * This function may return null on errors (errno).
 */
zzip__new__ ZZIP_DISK *
zzip_disk_new(void)
{
    ZZIP_DISK *disk = malloc(sizeof(ZZIP_DISK));
    if (! disk)
        return disk; /* ENOMEM */
    zzip_disk_init(disk, 0, 0);
    return disk;
}

/** turn a filehandle into a mmapped zip disk archive handle
 *
 * This function uses the given file-descriptor to detect the length of the
 * file and calls the system => mmap(2) to put it in main memory. If it is
 * successful then a newly allocated ZZIP_DISK* is returned with
 * disk->buffer pointing to the mapview of the zipdisk content.
 *
 * This function may return null on errors (errno).
 */
zzip__new__ ZZIP_DISK *
zzip_disk_mmap(int fd)
{
    struct stat st;
    if (fstat(fd, &st) || ! st.st_size)
        return 0; /* EACCESS */
    ___ ZZIP_DISK *disk = zzip_disk_new();
    if (! disk)
        return 0; /* ENOMEM */
    disk->buffer = _zzip_mmap(disk->mapped, fd, 0, st.st_size);
    if (disk->buffer == MAP_FAILED)
    { 
        free (disk); 
        return 0; /* EFAULT */
    }
    disk->endbuf = disk->buffer + st.st_size;
    return disk;
    ____;
}

/** => zzip_disk_mmap
 * This function is the inverse of => zzip_disk_mmap and using the system
 * munmap(2) on the buffer area and => free(3) on the ZZIP_DISK structure.
 */
int
zzip_disk_munmap(ZZIP_DISK * disk)
{
    if (! disk)
        return 0;
    _zzip_munmap(disk->mapped, disk->buffer, disk->endbuf - disk->buffer);
    free(disk);
    return 0;
}

/** => zzip_disk_mmap
 *
 * This function opens the given archive by name and turn the filehandle
 * to  => zzip_disk_mmap for bringing it to main memory. If it can not
 * be => mmap(2)'ed then we slurp the whole file into a newly => malloc(2)'ed
 * memory block. Only if that fails too then we return null. Since handling
 * of disk->buffer is ambiguous it should not be snatched away please.
 *
 * This function may return null on errors (errno).
 */
zzip__new__ ZZIP_DISK *
zzip_disk_open(char *filename)
{
#  ifndef O_BINARY
#  define O_BINARY 0
#  endif
    struct stat st;
    if (stat(filename, &st) || ! st.st_size)
        return 0; /* ENOENT */
    ___ int fd = open(filename, O_RDONLY | O_BINARY);
    if (fd <= 0)
        return 0; /* EACCESS */
    ___ ZZIP_DISK *disk = zzip_disk_mmap(fd);
    if (disk)
        return disk;
    ___ zzip_byte_t *buffer = malloc(st.st_size);
    if (! buffer)
    {
        return 0; /* ENOMEM */
    }
    if ((st.st_size < read(fd, buffer, st.st_size)))
    {
        free (buffer);
        return 0; /* EIO */
    }
    disk = zzip_disk_new();
    if (! disk)
    {
        free (buffer);
        return 0; /* ENOMEM */
    }
    disk->buffer = buffer;
    disk->endbuf = buffer + st.st_size;
    disk->mapped = -1;
    disk->flags |= ZZIP_DISK_FLAGS_OWNED_BUFFER;
    return disk;
    ____;
    ____;
    ____;
}

/** => zzip_disk_mmap
 * This function will attach a buffer with a zip image
 * that was acquired from another source than a file.
 * Note that if zzip_disk_mmap fails then zzip_disk_open
 * will fall back and try to read the full file to memory
 * wrapping a ZZIP_DISK around the memory buffer just as
 * this function will do. Note that this function will not
 * own the buffer, it will neither be written nor free()d.
 *
 * This function may return null (errno).
 */
zzip__new__ ZZIP_DISK *
zzip_disk_buffer(void *buffer, size_t buflen) {
    ZZIP_DISK *disk = zzip_disk_new();
    if (disk)
    {
        disk->buffer = (zzip_byte_t *) buffer;
        disk->endbuf = (zzip_byte_t *) buffer + buflen;
        disk->mapped = -1;
    }
    return disk;
}

/** => zzip_disk_mmap
 *
 * This function will release all data needed to access a (mmapped)
 * zip archive, including any malloc()ed blocks, sharedmem mappings
 * and it dumps the handle struct as well.
 *
 * This function returns 0 on success (or whatever => munmap says).
 */
int
zzip_disk_close(ZZIP_DISK * disk)
{
    if (! disk)
        return 0;
    if (disk->mapped != -1)
        return zzip_disk_munmap(disk);
    if (disk->flags & ZZIP_DISK_FLAGS_OWNED_BUFFER)
        free(disk->buffer);
    free(disk);
    return 0;
}

/* ====================================================================== */
/*                      helper functions                                  */


/** helper functions for (mmapped) zip access api
 *
 * This function augments the other zzip_disk_entry_* helpers: here we move
 * a disk_entry pointer (as returned by _find* functions) into a pointer to
 * the data block right after the file_header. Only disk->buffer would be
 * needed to perform the seek but we check the mmapped range end as well.
 *
 * This function returns a pointer into disk->buffer or 0 on error (errno).
 */
zzip_byte_t *
zzip_disk_entry_to_data(ZZIP_DISK * disk, struct zzip_disk_entry * entry)
{
    struct zzip_file_header *file = zzip_disk_entry_to_file_header(disk, entry);
    if (! file)
        return 0; /* EBADMSG */
    return zzip_file_header_to_data(file);
}

/** => zzip_disk_entry_to_data
 * This function does half the job of => zzip_disk_entry_to_data where it
 * can augment with => zzip_file_header_to_data helper from format/fetch.h
 *
 * This function returns a pointer into disk->buffer or 0 on error (errno).
 */
struct zzip_file_header *
zzip_disk_entry_to_file_header(ZZIP_DISK * disk, struct zzip_disk_entry *entry)
{
    zzip_byte_t *const ptr = disk->buffer + zzip_disk_entry_fileoffset(entry);
    if (disk->buffer > ptr || ptr >= disk->endbuf)
    {
        errno = EBADMSG;
        return 0;
    }
    ___  struct zzip_file_header *file_header = (void *) ptr;
    if (zzip_file_header_get_magic(file_header) != ZZIP_FILE_HEADER_MAGIC)
    {
        errno = EBADMSG;
        return 0;
    }
    return file_header;
    ____;
}

/** => zzip_disk_entry_to_data
 * This function is a big helper despite its little name: in a zip file the
 * encoded filenames are usually NOT zero-terminated but for common usage
 * with libc we need it that way. Secondly, the filename SHOULD be present
 * in the zip central directory but if not then we fallback to the filename
 * given in the file_header of each compressed data portion.
 *
 * This function returns a new string buffer, or null on error.
 * If no name can be found then an empty string is returned.
 */
zzip__new__ char *
zzip_disk_entry_strdup_name(ZZIP_DISK * disk, struct zzip_disk_entry *entry)
{
    if (! disk || ! entry)
    {
        errno=EINVAL;
        return 0;
    }

    ___ char *name = 0;
    zzip_size_t len = len = zzip_disk_entry_namlen(entry);
    if (len)
    {
        name = zzip_disk_entry_to_filename(entry);
    }
    else
    {
        struct zzip_file_header *file = zzip_disk_entry_to_file_header(disk, entry);
        if (! file)
           return 0; /* EBADMSG */

        len = zzip_file_header_namlen(file);
        if (! len)
        {
            /* neither a name in disk_entry nor in file_header */
            return strdup(""); /* ENOMEM */
        }
        name = zzip_file_header_to_filename(file);
    }

    if ((zzip_byte_t *) name < disk->buffer ||
        (zzip_byte_t *) name + len > disk->endbuf)
    {
        errno=EBADMSG;
        return 0;
    }

    return _zzip_strndup(name, len); /* ENOMEM */
    ____;
}

/** => zzip_disk_entry_to_data
 * This function is similar creating a reference to a zero terminated
 * string but it can only exist in the zip central directory entry.
 *
 * This function returns a new string buffer, or null on error (errno).
 * If no name can be found then an empty string is returned.
 */
zzip__new__ char *
zzip_disk_entry_strdup_comment(ZZIP_DISK * disk, struct zzip_disk_entry *entry)
{
    if (! disk || ! entry)
    {
        errno = EINVAL;
        return 0;
    }

    ___ zzip_size_t len = zzip_disk_entry_comment(entry);
    if (! len)
    {
        return strdup(""); /* ENOMEM */
    }

    ___ char *text = zzip_disk_entry_to_comment(entry);
    if ((zzip_byte_t *) text < disk->buffer ||
        (zzip_byte_t *) text + len > disk->endbuf) 
    {
        errno = EBADMSG;
        return 0;
    }

    return _zzip_strndup(text, len); /* ENOMEM */
    ____;
    ____;
}

/* ====================================================================== */

/** => zzip_disk_findfile
 *
 * This function is the first call of all the zip access functions here.
 * It contains the code to find the first entry of the zip central directory.
 * Here we require the mmapped block to represent a real zip file where the
 * disk_trailer is _last_ in the file area, so that its position would be at
 * a fixed offset from the end of the file area if not for the comment field
 * allowed to be of variable length (which needs us to do a little search
 * for the disk_tailer). However, in this simple implementation we disregard
 * any disk_trailer info telling about multidisk archives, so we just return
 * a pointer to the zip central directory.
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
 * the start of the zip central directory.So this function may return null
 * and sets errno.
 */
struct zzip_disk_entry *
zzip_disk_findfirst(ZZIP_DISK * disk)
{
    DBG1("findfirst");
    if (! disk)
    {
        DBG1("non arg");
        errno = EINVAL;
        return 0;
    }
    if (disk->buffer > disk->endbuf - sizeof(struct zzip_disk_trailer))
    {
        DBG1("not enough data for a disk trailer");
        errno = EBADMSG;
        return 0;
    }
    ___ zzip_byte_t *p = disk->endbuf - sizeof(struct zzip_disk_trailer);
    for (; p >= disk->buffer; p--)
    {
        zzip_byte_t *root;      /* (struct zzip_disk_entry*) */
	zzip_size_t rootsize;	/* Size of root central directory */

        if (zzip_disk_trailer_check_magic(p))
        {
            struct zzip_disk_trailer *trailer = (struct zzip_disk_trailer *) p;
            zzip_size_t rootseek = zzip_disk_trailer_get_rootseek(trailer);
	    rootsize = zzip_disk_trailer_get_rootsize(trailer);

            root = disk->buffer + rootseek;
            DBG2("disk rootseek at %lli", (long long)rootseek);
            if (root > p)
            {
                /* the first disk_entry is after the disk_trailer? can't be! */
                DBG2("have rootsize at %lli", (long long)rootsize);
                if (disk->buffer + rootsize > p)
                    continue;
                /* a common brokeness that can be fixed: we just assume the
                 * central directory was written directly before the trailer:*/
                root = p - rootsize;
            }
        } else if (zzip_disk64_trailer_check_magic(p))
        {
            struct zzip_disk64_trailer *trailer =
                (struct zzip_disk64_trailer *) p;
            if (sizeof(void *) < 8)
            {
                DBG1("disk64 trailer in non-largefile part");
                errno = EFBIG;
                return 0;
            }
            zzip_size_t rootseek = zzip_disk64_trailer_get_rootseek(trailer);
	    rootsize = zzip_disk64_trailer_get_rootsize(trailer);
            DBG2("disk64 rootseek at %lli", (long long)rootseek);
            root = disk->buffer + rootseek;
            if (root > p)
                continue;
        } else
        {
            continue;
        }

        DBG4("buffer %p root %p endbuf %p", disk->buffer, root, disk->endbuf);
        if (root < disk->buffer)
        {
            DBG1("root before buffer should be impossible");
            errno = EBADMSG;
            return 0;
        }
	if (root >= disk->endbuf || (root + rootsize) >= disk->endbuf)
	{
	    DBG1("root behind endbuf should be impossible");
	    errno = EBADMSG;
	    return 0;
	}
        if (zzip_disk_entry_check_magic(root))
        {
            DBG2("found the disk root %p", root);
            return (struct zzip_disk_entry *) root;
        }
    } ____;
    /* not found */
    errno = ENOENT;
    return 0;
}

/** => zzip_disk_findfile
 *
 * This function takes an existing disk_entry in the central root directory
 * (e.g. from zzip_disk_findfirst) and returns the next entry within in
 * the given bounds of the mmapped file area.
 *
 * This function returns null if no next entry can be found.
 * This function may return null on errors. (errno = ENOENT|EINVAL|EBADMSG)
 */
struct zzip_disk_entry *
zzip_disk_findnext(ZZIP_DISK * disk, struct zzip_disk_entry *entry)
{
    if (! disk || ! entry)
    {
        errno = EINVAL;
        return 0;
    }
    if ((zzip_byte_t *) entry < disk->buffer ||
        (zzip_byte_t *) entry > disk->endbuf - sizeof(entry) ||
        ! zzip_disk_entry_check_magic(entry) ||
        zzip_disk_entry_sizeto_end(entry) > 64 * 1024)
    {
        errno = EBADMSG;
        return 0;
    }
    entry = zzip_disk_entry_to_next_entry(entry);
    if ((zzip_byte_t *) entry > disk->endbuf - sizeof(entry) ||
        ! zzip_disk_entry_check_magic(entry) ||
        zzip_disk_entry_sizeto_end(entry) > 64 * 1024 ||
        zzip_disk_entry_skipto_end(entry) + sizeof(entry) > disk->endbuf)
    {
        errno = ENOENT;
        return 0;
    }
    return entry;
}

/** search for files in the (mmapped) zip central directory
 *
 * This function is given a filename as an additional argument, to find the
 * disk_entry matching a given filename. The compare-function is usually
 * strcmp or strcasecmp or perhaps strcoll, if null then strcmp is used.
 * - use null as argument for "after"-entry when searching the first
 * matching entry, otherwise the last returned value if you look for other
 * entries with a special "compare" function (if null then a doubled search
 * is rather useless with this variant of _findfile).
 *
 * This functionreturns the entry pointer.
 * This function may return null on error. (errno = ENOMEM|EBADMSG|ENOENT)
 */
struct zzip_disk_entry *
zzip_disk_findfile(ZZIP_DISK * disk, char *filename,
                   struct zzip_disk_entry *after, zzip_strcmp_fn_t compare)
{
    struct zzip_disk_entry *entry = (! after ? zzip_disk_findfirst(disk)
                                     : zzip_disk_findnext(disk, after));
    if (! compare)
        compare = (zzip_strcmp_fn_t) ((disk->flags & ZZIP_DISK_FLAGS_MATCH_NOCASE) ?
                                      (_zzip_strcasecmp) : (strcmp));
    for (; entry; entry = zzip_disk_findnext(disk, entry))
    {
        /* filenames within zip files are often not null-terminated! */
        char *realname = zzip_disk_entry_strdup_name(disk, entry);
        if (! realname)
        {
            return 0; /* ENOMEM | EBADMSG */
        }
        if (! compare(filename, realname))
        {
            free(realname);
            return entry; /* found */
        }
        free(realname);
    }
    errno = ENOENT;
    return 0;
}

/** => zzip_disk_findfile
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
 * next entry matching the given filespec.
 *
 * This function will return the matching entry pointer.
 * This function may return null on error. (errno = ENOMEM|EBADMSG|ENOENT)
 */
struct zzip_disk_entry *
zzip_disk_findmatch(ZZIP_DISK * disk, char *filespec,
                    struct zzip_disk_entry *after,
                    zzip_fnmatch_fn_t compare, int flags)
{
    struct zzip_disk_entry *entry = (! after ? zzip_disk_findfirst(disk)
                                     : zzip_disk_findnext(disk, after));
    if (! compare)
    {
        compare = (zzip_fnmatch_fn_t) _zzip_fnmatch;
        if (disk->flags & ZZIP_DISK_FLAGS_MATCH_NOCASE)
            flags |= _zzip_FNM_CASEFOLD;
    }
    for (; entry; entry = zzip_disk_findnext(disk, entry))
    {
        /* filenames within zip files are often not null-terminated! */
        char *realname = zzip_disk_entry_strdup_name(disk, entry);
        if (! realname)
        {
            return 0; /* ENOMEM | EBADMSG */
        }
        if (compare(filespec, realname, flags))
        {
            free(realname);
            return entry; /* found */
        }
        free(realname);
    }
    errno = ENOENT;
    return 0;
}

/* ====================================================================== */

/** => zzip_disk_fopen
 *
 * the ZZIP_DISK_FILE* is rather simple in just encapsulating the
 * arguments given to this function plus a zlib deflate buffer.
 * Note that the ZZIP_DISK pointer does already contain the full
 * mmapped file area of a zip disk, so open()ing a file part within
 * that area happens to be a lookup of its bounds and encoding. That
 * information is memorized on the ZZIP_DISK_FILE so that subsequent
 * _read() operations will be able to get the next data portion or
 * return an eof condition for that file part wrapped in the zip archive.
 *
 * This function may return null on errors (errno = ENOMEM|EBADMSG).
 */
zzip__new__ ZZIP_DISK_FILE *
zzip_disk_entry_fopen(ZZIP_DISK * disk, ZZIP_DISK_ENTRY * entry)
{
    /* keep this in sync with zzip_mem_entry_fopen */
    struct zzip_file_header *header =
        zzip_disk_entry_to_file_header(disk, entry);
    if (! header)
        return 0; /* EBADMSG */
    ___ ZZIP_DISK_FILE *file = malloc(sizeof(ZZIP_DISK_FILE));
    if (! file)
        return 0; /* ENOMEM */
    file->buffer = disk->buffer;
    file->endbuf = disk->endbuf;
    file->avail = zzip_file_header_usize(header);

    if (! file->avail || zzip_file_header_data_stored(header))
    { 
         file->stored = zzip_file_header_to_data (header);
         DBG2("stored size %i", (int) file->avail);
         if (file->stored + file->avail >= disk->endbuf)
             goto error;
         return file; 
    }

    file->stored = 0;
    file->zlib.opaque = 0;
    file->zlib.zalloc = Z_NULL;
    file->zlib.zfree = Z_NULL;
    file->zlib.avail_in = zzip_file_header_csize(header);
    file->zlib.next_in = zzip_file_header_to_data(header);

    DBG2("compressed size %i", (int) file->zlib.avail_in);
    if (file->zlib.next_in + file->zlib.avail_in >= disk->endbuf)
         goto error;
    if (file->zlib.next_in < disk->buffer)
         goto error;

    if (! zzip_file_header_data_deflated(header))
        goto error;
    if (inflateInit2(&file->zlib, -MAX_WBITS) != Z_OK)
        goto error;

    return file;
error:
    free (file);
    errno = EBADMSG;
    return 0; 
    ____;
}

/** openening a file part wrapped within a (mmapped) zip archive
 *
 * This function opens a file found by name, so it does a search into
 * the zip central directory with => zzip_disk_findfile and whatever
 * is found first is given to => zzip_disk_entry_fopen
 *
 * This function may return null on errors (errno).
 */
zzip__new__ ZZIP_DISK_FILE *
zzip_disk_fopen(ZZIP_DISK * disk, char *filename)
{
    ZZIP_DISK_ENTRY *entry = zzip_disk_findfile(disk, filename, 0, 0);
    if (! entry)
        return 0; /* EBADMSG */
    return zzip_disk_entry_fopen(disk, entry);
}

/** => zzip_disk_fopen
 *
 * This function reads more bytes into the output buffer specified as
 * arguments. The return value is null on eof or error, the stdio-like
 * interface can not distinguish between these so you need to check
 * with => zzip_disk_feof for the difference.
 */
zzip_size_t
zzip_disk_fread(void *ptr, zzip_size_t sized, zzip_size_t nmemb,
                ZZIP_DISK_FILE * file)
{
    zzip_size_t size = sized * nmemb;
    if (! ptr || ! sized || ! file)
        return 0;
    if (size > file->avail)
        size = file->avail;
    if (file->stored)
    {
        if (file->stored + size >= file->endbuf)
        {
            DBG1("try to read beyond end of file");
            return 0; /* ESPIPE */
        }
        DBG3("copy stored %p %i", file->stored, (int)size);
        memcpy(ptr, file->stored, size);
        file->stored += size;
        file->avail -= size;
        return size;
    }

    file->zlib.avail_out = sized * nmemb;
    file->zlib.next_out = ptr;
    ___ zzip_size_t total_old = file->zlib.total_out;
    ___ int err = inflate(&file->zlib, Z_NO_FLUSH);
    if (err == Z_STREAM_END)
        file->avail = 0;
    else if (err == Z_OK)
        file->avail -= file->zlib.total_out - total_old;
    else
        return 0;
    return file->zlib.total_out - total_old;
    ____;
    ____;
}

/** => zzip_disk_fopen
 * This function releases any zlib decoder info needed for decompression
 * and dumps the ZZIP_DISK_FILE* then.
 *
 * This function always returns 0.
 */
int
zzip_disk_fclose(ZZIP_DISK_FILE * file)
{
    if (file)
    {
        if (! file->stored)
            inflateEnd(&file->zlib);
        free(file);
    }
    return 0;
}

/** => zzip_disk_fopen
 *
 * This function allows to distinguish an error from an eof condition.
 * Actually, if we found an error but we did already reach eof then we
 * just keep on saying that it was an eof, so the app can just continue.
 *
 * This function returns EOF in case and 0 when not at the end
 * of file.
 */
int
zzip_disk_feof(ZZIP_DISK_FILE * file)
{
    if (! file || ! file->avail)
        return EOF;
    return 0;
}
