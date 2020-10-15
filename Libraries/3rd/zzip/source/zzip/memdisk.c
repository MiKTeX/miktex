
/*
 * NOTE: this is part of libzzipmmapped (i.e. it is not libzzip).
 *                                            ==================
 *
 * The mem_disk cache will parse the central information of a zip archive
 * and store it internally. One the one hand it allows to find files
 * faster - no disk access is required and endian conversion is not
 * needed. If zzip is compiled with zip extensions then it is about
 * the only way to build maintainable code around the zip format.
 *
 * Note that 64bit support is almost entirely living in extension
 * blocks as well as different character encodings and file access
 * control bits that are mostly platform specific.
 *
 * Author:
 *    Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) Guido Draheim, use under copyleft (LGPL,MPL)
 */
#define _ZZIP_DISK_FILE_STRUCT 1

#include <zzip/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <zlib.h>
#include <zzip/format.h>
#include <zzip/fetch.h>
#include <zzip/mmapped.h>
#include <zzip/memdisk.h>
#include <zzip/__fnmatch.h>
#include <zzip/__errno.h>

#define ___ {
#define ____ }

#define DEBUG 1
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

#define ZZIP_EXTRA_zip64 0x0001
typedef struct _zzip_extra_zip64
{                               /* ZIP64 extended information extra field */
    zzip_byte_t z_datatype[2];  /* Tag for this "extra" block type */
    zzip_byte_t z_datasize[2];  /* Size of this "extra" block */
    zzip_byte_t z_usize[8];     /* Original uncompressed file size */
    zzip_byte_t z_csize[8];     /* Size of compressed data */
    zzip_byte_t z_offset[8];    /* Offset of local header record */
    zzip_byte_t z_diskstart[4]; /* Number of the disk for file start */
} zzip_extra_zip64;

/*forward*/

static zzip__new__ ZZIP_MEM_ENTRY *
zzip_mem_entry_new(ZZIP_DISK * disk, ZZIP_DISK_ENTRY * entry);
static void
zzip_mem_entry_free(ZZIP_MEM_ENTRY * _zzip_restrict item);

/** => zzip_mem_disk_open
 * This function is internally used to prepare opening a disk.
 */
zzip__new__ ZZIP_MEM_DISK *
zzip_mem_disk_new(void)
{
    return calloc(1, sizeof(ZZIP_MEM_DISK));
}

/** create new diskdir handle.
 *  This function wraps underlying zzip_disk_open. 
 */
zzip__new__ ZZIP_MEM_DISK *
zzip_mem_disk_open(char *filename)
{
    ZZIP_DISK *disk = zzip_disk_open(filename);
    if (! disk)
    { 
       debug2("can not open disk file %s", filename);
       return 0;
    }
    ___ ZZIP_MEM_DISK *dir = zzip_mem_disk_new();
    if (zzip_mem_disk_load(dir, disk) == -1)
    {
       debug2("unable to load disk %s", filename);
    }
    return dir;
    ____;
}

/** => zzip_mem_disk_open
 *  This function wraps the underlying zzip_disk_open. 
 */
zzip__new__ ZZIP_MEM_DISK *
zzip_mem_disk_fdopen(int fd)
{
    ZZIP_DISK *disk = zzip_disk_mmap(fd);
    if (! disk)
    { 
       debug2("can not open disk fd %i", fd);
       return 0;
    }
    ___ ZZIP_MEM_DISK *dir = zzip_mem_disk_new();
    if (zzip_mem_disk_load(dir, disk) == -1)
    {
       debug2("unable to load disk fd %i", fd);
    }
    return dir;
    ____;
}

/** create new diskdir handle.
 *  This function wraps underlying zzip_disk_buffer.
 */
zzip__new__ ZZIP_MEM_DISK *
zzip_mem_disk_buffer(char *buffer, size_t buflen)
{
    ZZIP_DISK *disk = zzip_disk_buffer(buffer, buflen);
    if (! disk)
    { 
       debug2("can not open disk buf %p", buffer);
       return 0;
    }
    ___ ZZIP_MEM_DISK *dir = zzip_mem_disk_new();
    if (zzip_mem_disk_load(dir, disk) == -1)
    {
       debug2("unable to load disk buf %p", buffer);
    }
    return dir;
    ____;
}

/** parse central dir.
 *  creates an internal copy of each entry converted to the local platform.
 *  returns: number of entries, or -1 on error (setting errno)
 */
long
zzip_mem_disk_load(ZZIP_MEM_DISK * dir, ZZIP_DISK * disk)
{
    if (! dir || ! disk)
        { errno=EINVAL; return -1; }
    if (dir->list)
        zzip_mem_disk_unload(dir);
    ___ long count = 0;
    ___ struct zzip_disk_entry *entry = zzip_disk_findfirst(disk);
    if (!entry) goto error;
    for (; entry; entry = zzip_disk_findnext(disk, entry))
    {
        ZZIP_MEM_ENTRY *item = zzip_mem_entry_new(disk, entry);
        if (! item)
        {
            debug1("unable to load entry");
            goto error;
        }
        if (dir->last)
        {
            dir->last->zz_next = item;  /* chain last */
        } else
        {
            dir->list = item;
        }
        dir->last = item;       /* to earlier */
        count++;
    }
    ____;
    dir->disk = disk;
    return count;
    ____;
  error:
    zzip_mem_disk_unload(dir);
    return -1;
}

/** convert a zip disk entry to internal format.
 * creates a new item parsing the information out of the various places
 * in the zip archive. This is a good place to extend functionality if
 * you have a project with extra requirements as you can push more bits
 * right into the diskdir_entry for later usage in higher layers.
 * returns: new item, or null on error (setting errno =  ENOMEM|EBADMSG)
 */
zzip__new__ ZZIP_MEM_ENTRY *
zzip_mem_entry_new(ZZIP_DISK * disk, ZZIP_DISK_ENTRY * entry)
{
    if (! disk || ! entry)
        { errno=EINVAL; return 0; }
    ___ ZZIP_MEM_ENTRY *item = calloc(1, sizeof(*item));
    if (! item)
        return 0;               /* errno=ENOMEM; */
    ___ struct zzip_file_header *header =
        zzip_disk_entry_to_file_header(disk, entry);
    if (! header) 
    {
        debug1("no header in entry");
        free (item);
        return 0; /* errno=EBADMSG; */
    }
    /*  there is a number of duplicated information in the file header
     *  or the disk entry block. Theoretically some part may be missing
     *  that exists in the other, ... but we will prefer the disk entry.
     */
    item->zz_comment = zzip_disk_entry_strdup_comment(disk, entry);
    item->zz_name = zzip_disk_entry_strdup_name(disk, entry);
    item->zz_data = zzip_file_header_to_data(header);
    item->zz_flags = zzip_disk_entry_get_flags(entry);
    item->zz_compr = zzip_disk_entry_get_compr(entry);
    item->zz_mktime = zzip_disk_entry_get_mktime(entry);
    item->zz_crc32 = zzip_disk_entry_get_crc32(entry);
    item->zz_csize = zzip_disk_entry_get_csize(entry);
    item->zz_usize = zzip_disk_entry_get_usize(entry);
    item->zz_diskstart = zzip_disk_entry_get_diskstart(entry);
    item->zz_filetype = zzip_disk_entry_get_filetype(entry);

    /*
     * If zz_data+zz_csize exceeds the size of the file, bail out
     */
    if ((item->zz_data + item->zz_csize) < disk->buffer ||
	(item->zz_data + item->zz_csize) >= disk->endbuf)
    {
	goto error;
    }
   /*
     * If the file is uncompressed, zz_csize and zz_usize should be the same
     * If they are not, we cannot guarantee that either is correct, so ...
     */
    if (item->zz_compr == ZZIP_IS_STORED && item->zz_csize != item->zz_usize)
    {
        goto error;
    }
    /* zz_comment and zz_name are empty strings if not present on disk */
    if (! item->zz_comment || ! item->zz_name)
    {
        goto error; /* errno=ENOMEM */
    }

    {   /* copy the extra blocks to memory as well (maximum 64K each) */
        zzip_size_t /*           */ ext1_len = zzip_disk_entry_get_extras(entry);
        zzip_byte_t *_zzip_restrict ext1_ptr = zzip_disk_entry_to_extras(entry);
        zzip_size_t /*           */ ext2_len = zzip_file_header_get_extras(header);
        zzip_byte_t *_zzip_restrict ext2_ptr = zzip_file_header_to_extras(header);

        if (ext1_len > 0 && ext1_len <= 65535)
        {
            if (ext1_ptr + ext1_len >= disk->endbuf)
            {
                errno = EBADMSG;
                goto error; /* zzip_mem_entry_free(item); return 0; */
            } else {
                void *mem = malloc(ext1_len);
                if (! mem) goto error; /* errno = ENOMEM */
                item->zz_ext[1] = mem;
                item->zz_extlen[1] = ext1_len;
                memcpy(mem, ext1_ptr, ext1_len);
            }
        }
        if (ext2_len > 0 && ext2_len <= 65535)
        {
            if (ext2_ptr + ext2_len >= disk->endbuf)
            {
                errno = EBADMSG;
                goto error; /* zzip_mem_entry_free(item); return 0; */
            } else {
                void *mem = malloc(ext2_len);
                if (! mem) goto error; /* errno = ENOMEM */
                item->zz_ext[2] = mem;
                item->zz_extlen[2] = ext2_len;
                memcpy(mem, ext2_ptr, ext2_len);
            }
        }
    }
    {
        /* override sizes/offsets with zip64 values for largefile support */
        zzip_extra_zip64 *block = (zzip_extra_zip64 *)
            zzip_mem_entry_find_extra_block(item, ZZIP_EXTRA_zip64, sizeof(zzip_extra_zip64));
        if (block)
        {
            item->zz_usize = ZZIP_GET64(block->z_usize);
            item->zz_csize = ZZIP_GET64(block->z_csize);
            item->zz_offset = ZZIP_GET64(block->z_offset);
            item->zz_diskstart = ZZIP_GET32(block->z_diskstart);
        }
    }
    /* NOTE:
     * All information from the central directory entry is now in memory.
     * Effectivly that allows us to modify it and write it back to disk.
     */
    return item;
    ____;
error:
    zzip_mem_entry_free(item);
    return 0;
    ____;
}

/** => zzip_mem_entry_find_extra_block.
 *
 * Note that for this function only the block_header is asserted 
 * to be completely in memory, so the returned pointer should be checked.
 */
ZZIP_EXTRA_BLOCK *
zzip_mem_entry_extra_block(ZZIP_MEM_ENTRY * entry, short datatype)
{
   return zzip_mem_entry_find_extra_block(entry, datatype, 16);
}

/** get extra block.
 * This function finds an extra block for the given datatype code.
 * The returned EXTRA_BLOCK is still in disk-encoding but
 * already a pointer into an allocated heap space block.
 *
 * The second argument of this function ensures that the 
 * complete datasize is in memory.
 */
ZZIP_EXTRA_BLOCK *
zzip_mem_entry_find_extra_block(ZZIP_MEM_ENTRY * entry, short datatype, zzip_size_t blocksize)
{
    int i = 2;
    while (1)
    {
        char* ext = (char*)( entry->zz_ext[i] );
        char* ext_end = ext + entry->zz_extlen[i];
        if (ext)
        {
	    /*
	     * Make sure that
	     * 1) the extra block header
	     * AND
	     * 2) the block we're looking for
	     * fit into the extra block!
	     */
            while (ext + zzip_extra_block_headerlength + blocksize <= ext_end)
            {
                if (datatype == zzip_extra_block_get_datatype(ext))
                {
                    if (blocksize <= zzip_extra_block_get_datasize(ext) + zzip_extra_block_headerlength)
                    {
                        return ((ZZIP_EXTRA_BLOCK*) ext);
                    }
                }
                /* skip to start of next extra_block */
                ___ zzip_size_t datasize = zzip_extra_block_get_datasize(ext);
                ext += zzip_extra_block_headerlength;
                ext += datasize;
                ____;
            }
        }
        if (! i)
            return 0;
        i--;
    }
}

/** => zzip_mem_disk_close
 * This function ends usage of a file entry in a disk.
 */
void
zzip_mem_entry_free(ZZIP_MEM_ENTRY * _zzip_restrict item)
{
    if (item)
    {
	/* *INDENT-OFF* */
	if (item->zz_ext[0]) free (item->zz_ext[0]);
	if (item->zz_ext[1]) free (item->zz_ext[1]);
	if (item->zz_ext[2]) free (item->zz_ext[2]);
	if (item->zz_comment) free (item->zz_comment);
	if (item->zz_name) free (item->zz_name);
	free (item);
	/* *INDENT-ON* */
    }
}

/** => zzip_mem_disk_close
 * This function will trigger an underlying disk_close 
 */
void
zzip_mem_disk_unload(ZZIP_MEM_DISK * dir)
{
    ZZIP_MEM_ENTRY *item = dir->list;
    while (item)
    {
        ZZIP_MEM_ENTRY *next = item->zz_next;
        zzip_mem_entry_free(item);
        item = next;
    }
    dir->list = dir->last = 0;
    zzip_disk_close(dir->disk);
    dir->disk = 0;
}

/** end usage.
 * This function closes the dir and disk handles.
 */
void
zzip_mem_disk_close(ZZIP_MEM_DISK * _zzip_restrict dir)
{
    if (dir)
    {
        zzip_mem_disk_unload(dir);
        zzip_disk_close(dir->disk);
        free(dir);
    }
}

#if 0
static void
foo(short zz_datatype)
{
    /* Header IDs of 0 through 31 are reserved for use by PKWARE.(APPNOTE.TXT) */
    switch (zz_datatype)
    {
    /* *INDENT-OFF* */
    case 0x0001: /* ZIP64 extended information extra field */
    case 0x0007: /* AV Info */
    case 0x0008: /* Reserved for future Unicode file name data (PFS) */
    case 0x0009: /* OS/2 */
    case 0x000a: /* NTFS */
    case 0x000c: /* OpenVMS */
    case 0x000d: /* Unix */
    case 0x000e: /* Reserved for file stream and fork descriptors */
    case 0x000f: /* Patch Descriptor */
    case 0x0014: /* PKCS#7 Store for X.509 Certificates */
    case 0x0015: /* X.509 Certificate ID and Signature for file */
    case 0x0016: /* X.509 Certificate ID for Central Directory */
    case 0x0017: /* Strong Encryption Header */
    case 0x0018: /* Record Management Controls */
    case 0x0019: /* PKCS#7 Encryption Recipient Certificate List */
    /* ......................................................... */
    case 0x0065: /* IBM S/390, AS/400 attributes - uncompressed */
    case 0x0066: /* Reserved for IBM S/390, AS/400 attr - compressed */
    case 0x07c8: /* Macintosh */
    case 0x2605: /* ZipIt Macintosh */
    case 0x2705: /* ZipIt Macintosh 1.3.5+ */
    case 0x2805: /* ZipIt Macintosh 1.3.5+ */
    case 0x334d: /* Info-ZIP Macintosh */
    case 0x4341: /* Acorn/SparkFS  */
    case 0x4453: /* Windows NT security descriptor (binary ACL) */
    case 0x4704: /* VM/CMS */
    case 0x470f: /* MVS */
    case 0x4b46: /* FWKCS MD5 (see below) */
    case 0x4c41: /* OS/2 access control list (text ACL) */
    case 0x4d49: /* Info-ZIP OpenVMS */
    case 0x4f4c: /* Xceed original location extra field */
    case 0x5356: /* AOS/VS (ACL) */
    case 0x5455: /* extended timestamp */
    case 0x554e: /* Xceed unicode extra field */
    case 0x5855: /* Info-ZIP Unix (original, also OS/2, NT, etc) */
    case 0x6542: /* BeOS/BeBox */
    case 0x756e: /* ASi Unix */
    case 0x7855: /* Info-ZIP Unix (new) */
    case 0xfd4a: /* SMS/QDOS */
    /* *INDENT-ON* */
    }
}
#endif

/** search entries.
 * This function walks through the zip directory looking for a file.
 */
ZZIP_MEM_ENTRY *
zzip_mem_disk_findfile(ZZIP_MEM_DISK * dir,
                       char *filename, ZZIP_MEM_ENTRY * after,
                       zzip_strcmp_fn_t compare)
{
    ZZIP_MEM_ENTRY *entry = (! after ? dir->list : after->zz_next);
    if (! compare)
        compare = (zzip_strcmp_fn_t) (strcmp);
    for (; entry; entry = entry->zz_next)
    {
        if (! compare(filename, entry->zz_name))
        {
            return entry;
        }
    }
    return 0;
}

/* => zzip_mem_disk_findfile
 * This function uses an fnmatch-like comparator to find files.
 */
ZZIP_MEM_ENTRY *
zzip_mem_disk_findmatch(ZZIP_MEM_DISK * dir,
                        char *filespec, ZZIP_MEM_ENTRY * after,
                        zzip_fnmatch_fn_t compare, int flags)
{
    ZZIP_MEM_ENTRY *entry = (! after ? dir->list : after->zz_next);
    if (! compare)
        compare = (zzip_fnmatch_fn_t) _zzip_fnmatch;
    for (; entry; entry = entry->zz_next)
    {
        if (! compare(filespec, entry->zz_name, flags))
        {
            return entry;
        }
    }
    return 0;
}

/** start usage.
 * This function opens a referenced file entry from a openend disk.
 */
zzip__new__ ZZIP_MEM_DISK_FILE *
zzip_mem_entry_fopen(ZZIP_MEM_DISK * dir, ZZIP_MEM_ENTRY * entry)
{
    /* keep this in sync with zzip_disk_entry_fopen */
    ZZIP_DISK_FILE *file = malloc(sizeof(ZZIP_MEM_DISK_FILE));
    if (! file)
        return file;
    file->buffer = dir->disk->buffer;
    file->endbuf = dir->disk->endbuf;
    file->avail = zzip_mem_entry_usize(entry);

    if (! file->avail || zzip_mem_entry_data_stored(entry))
        { file->stored = zzip_mem_entry_to_data (entry); return file; }

    file->stored = 0;
    file->zlib.opaque = 0;
    file->zlib.zalloc = Z_NULL;
    file->zlib.zfree = Z_NULL;
    file->zlib.avail_in = zzip_mem_entry_csize(entry);
    file->zlib.next_in = zzip_mem_entry_to_data(entry);

    debug2("compressed size %i", (int) file->zlib.avail_in);
    if (file->zlib.next_in + file->zlib.avail_in >= file->endbuf)
         goto error;
    if (file->zlib.next_in < file->buffer)
         goto error;

    if (! zzip_mem_entry_data_deflated(entry) ||
        inflateInit2(&file->zlib, -MAX_WBITS) != Z_OK)
        { free (file); return 0; }

    return file;
error:
    errno = EBADMSG;
    return NULL;
}

/** => zzip_mem_disk_open
 * This function opens a file by name from an openend disk.
 */
zzip__new__ ZZIP_MEM_DISK_FILE *
zzip_mem_disk_fopen(ZZIP_MEM_DISK * dir, char *filename)
{
    ZZIP_MEM_ENTRY *entry = zzip_mem_disk_findfile(dir, filename, 0, 0);
    if (! entry)
        return 0;
    else
        return zzip_mem_entry_fopen(dir, entry);
}

/** get data
 * This function mimics the fread(2) behaviour.
 */
_zzip_size_t
zzip_mem_disk_fread(void *ptr, _zzip_size_t size, _zzip_size_t nmemb,
                    ZZIP_MEM_DISK_FILE * file)
{
    return zzip_disk_fread(ptr, size, nmemb, file);
}

/** close disk.
 * This function mimics the fclose(2) behaviour.
 */
int
zzip_mem_disk_fclose(ZZIP_MEM_DISK_FILE * file)
{
    return zzip_disk_fclose(file);
}

/** check disk.
 * This function mimics the feof(2) behaviour.
 */
int
zzip_mem_disk_feof(ZZIP_MEM_DISK_FILE * file)
{
    return zzip_disk_feof(file);
}

/** helper.
 * This function converts a zip dostime of an entry to unix time_t 
 */
long
zzip_disk_entry_get_mktime(ZZIP_DISK_ENTRY * entry)
{
    uint16_t dostime = ZZIP_GET16(entry->z_dostime.time);
    uint16_t dosdate = ZZIP_GET16(entry->z_dostime.date);
    struct tm date;
    date.tm_sec = (dostime) & 0x1F;     /* bits 0..4 */
    date.tm_min = (dostime >> 5) & 0x3F;        /* bits 5..10 */
    date.tm_hour = (dostime >> 11);     /* bits 11..15 */
    date.tm_mday = (dosdate) & 0x1F;    /* bits 16..20 */
    date.tm_mon = (dosdate >> 5) & 0xF; /* bits 21..24 */
    date.tm_year = (dosdate >> 9) + 80; /* bits 25..31 */
    return mktime(&date);       /* well, unix has that function... */
}
