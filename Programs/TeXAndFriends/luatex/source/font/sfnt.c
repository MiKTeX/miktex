/*

Copyright 2002 by Jin-Hwan Cho and Shunsaku Hirata,
the dvipdfmx project team <dvipdfmx@@project.ktug.or.kr>
Copyright 2006-2008 Taco Hoekwater <taco@@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

/*tex

    The code below is originally based on |dvipdfmx-0.13.2c|:

*/

#include "ptexlib.h"

#if HAVE_CONFIG_H
#  include <w2c/config.h>
#endif

#include <string.h>

#include "font/sfnt.h"

/*tex

    Types:

    \starttabulate[||||]
    \NC \type {true} \NC 0x74727565 \NC (Mac) TrueType  \NC \NR
    \NC \type {typ1| \NC 0x74797031 \NC (Mac) PostScript font housed in a sfnt wrapper \NC \NR
    \NC              \NC 0x00010000 \NC TrueType (Win)/OpenType \NC \NR
    \NC \type {OTTO} \NC            \NC PostScript CFF font with OpenType wrapper \NC \NR
    \NC \type {ttcf} \NC            \NC TrueType Collection \NC \NR
    \stoptabulate

*/

#define SFNT_TRUETYPE   0x00010000UL
#define SFNT_MAC_TRUE   0x74727565UL
#define SFNT_OPENTYPE   0x00010000UL
#define SFNT_POSTSCRIPT 0x4f54544fUL
#define SFNT_TTC        0x74746366UL

sfnt *sfnt_open(unsigned char *buff, int buflen)
{
    sfnt *sfont;
    ULONG type;
    sfont = xmalloc(sizeof(sfnt));
    sfont->loc = 0;
    sfont->buffer = buff;
    sfont->buflen = buflen;
    type = sfnt_get_ulong(sfont);
    if (type == SFNT_TRUETYPE || type == SFNT_MAC_TRUE) {
        sfont->type = SFNT_TYPE_TRUETYPE;
    } else if (type == SFNT_OPENTYPE) {
        sfont->type = SFNT_TYPE_OPENTYPE;
    } else if (type == SFNT_POSTSCRIPT) {
        sfont->type = SFNT_TYPE_POSTSCRIPT;
    } else if (type == SFNT_TTC) {
        sfont->type = SFNT_TYPE_TTC;
    }
    sfont->loc = 0;
    sfont->directory = NULL;
    return sfont;
}

static void release_directory(struct sfnt_table_directory *td)
{
    long i;
    if (td) {
        if (td->tables) {
            for (i = 0; i < td->num_tables; i++) {
                if (td->tables[i].data)
                    RELEASE(td->tables[i].data);
            }
            RELEASE(td->tables);
        }
        if (td->flags)
            RELEASE(td->flags);
        RELEASE(td);
    }
    return;
}

void sfnt_close(sfnt * sfont)
{
    if (sfont) {
        if (sfont->directory)
            release_directory(sfont->directory);
        RELEASE(sfont);
    }
    return;
}

int put_big_endian(void *s, LONG q, int n)
{
    int i;
    char *p;
    p = (char *) s;
    for (i = n - 1; i >= 0; i--) {
        p[i] = (char) (q & 0xff);
        q >>= 8;
    }
    return n;
}

/*tex

    Convert four-byte number to big endianess in a machine independent way.

*/

static void convert_tag(char *tag, unsigned long u_tag)
{
    int i;
    for (i = 3; i >= 0; i--) {
        tag[i] = (char) (u_tag % 256);
        u_tag /= 256;
    }
    return;
}

/*tex

    Computes the max power of $|2 <= n$:

*/

static unsigned max2floor(unsigned n)
{
    int val = 1;
    while (n > 1) {
        n /= 2;
        val *= 2;
    }
    return (unsigned) val;
}

/*tex

    Computes the log2 of the max power of $2 <= n$

*/

static unsigned log2floor(unsigned n)
{
    unsigned val = 0;
    while (n > 1) {
        n /= 2;
        val++;
    }
    return val;
}

static ULONG sfnt_calc_checksum(void *data, ULONG length)
{
    ULONG chksum = 0;
    BYTE *p, *endptr;
    ULONG count = 0;
    p = (BYTE *) data;
    endptr = p + length;
    while (p < endptr) {
        chksum = chksum + (ULONG) (p[0] << (8 * (3 - count)));
        count = ((count + 1) & 3);
        p++;
    }
    return chksum;
}

static int find_table_index(struct sfnt_table_directory *td, const char *tag)
{
    int idx;
    if (!td)
        return -1;
    for (idx = 0; idx < td->num_tables; idx++) {
        if (!memcmp(td->tables[idx].tag, tag, 4))
            return idx;
    }
    return -1;
}

void sfnt_set_table(sfnt * sfont, const char *tag, void *data, ULONG length)
{
    struct sfnt_table_directory *td;
    int idx;
    td = sfont->directory;
    idx = find_table_index(td, tag);
    if (idx < 0) {
        idx = td->num_tables;
        td->num_tables++;
        td->tables = RENEW(td->tables, td->num_tables, struct sfnt_table);
        memcpy(td->tables[idx].tag, tag, 4);
    }
    td->tables[idx].check_sum = sfnt_calc_checksum(data, length);
    td->tables[idx].offset = 0L;
    td->tables[idx].length = length;
    td->tables[idx].data = data;
    return;
}

ULONG sfnt_find_table_len(sfnt * sfont, const char *tag)
{
    ULONG length;
    struct sfnt_table_directory *td;
    int idx;
    td = sfont->directory;
    idx = find_table_index(td, tag);
    if (idx < 0)
        length = 0;
    else {
        length = td->tables[idx].length;
    }
    return length;
}

ULONG sfnt_find_table_pos(sfnt * sfont, const char *tag)
{
    ULONG offset;
    struct sfnt_table_directory *td;
    int idx;
    td = sfont->directory;
    idx = find_table_index(td, tag);
    if (idx < 0)
        offset = 0;
    else {
        offset = td->tables[idx].offset;
    }
    return offset;
}

ULONG sfnt_locate_table(sfnt * sfont, const char *tag)
{
    ULONG offset;
    offset = sfnt_find_table_pos(sfont, tag);
    if (offset == 0) {
        formatted_error("ttf","%s table not found",tag);
    }
    sfnt_seek_set(sfont, (long) offset);
    return offset;
}

int sfnt_read_table_directory(sfnt * sfont, ULONG offset)
{
    struct sfnt_table_directory *td;
    unsigned long i, u_tag;
    if (sfont->directory)
        release_directory(sfont->directory);
    sfont->directory = td = NEW(1, struct sfnt_table_directory);
    sfnt_seek_set(sfont, (long) offset);
    td->version = sfnt_get_ulong(sfont);
    td->num_tables = sfnt_get_ushort(sfont);
    td->search_range = sfnt_get_ushort(sfont);
    td->entry_selector = sfnt_get_ushort(sfont);
    td->range_shift = sfnt_get_ushort(sfont);
    td->flags = NEW(td->num_tables, char);
    td->tables = NEW(td->num_tables, struct sfnt_table);
    for (i = 0; i < td->num_tables; i++) {
        u_tag = sfnt_get_ulong(sfont);
        convert_tag(td->tables[i].tag, u_tag);
        td->tables[i].check_sum = sfnt_get_ulong(sfont);
        td->tables[i].offset = sfnt_get_ulong(sfont);
        td->tables[i].length = sfnt_get_ulong(sfont);
        td->tables[i].data = NULL;
        td->flags[i] = 0;
    }
    td->num_kept_tables = 0;
    return 0;
}

int sfnt_require_table(sfnt * sfont, const char *tag, int must_exist)
{
    struct sfnt_table_directory *td;
    int idx;
    td = sfont->directory;
    idx = find_table_index(td, tag);
    if (idx < 0) {
        if (must_exist)
            return -1;
    } else {
        td->flags[idx] |= SFNT_TABLE_REQUIRED;
        td->num_kept_tables++;
    }
    return 0;
}

/*tex

    All tables begin on four byte boundries, and pad any remaining space between
    tables with zeros. Entries in the Table Directory must be sorted in ascending
    order by tag The head table contains checksum of the whole font file. To
    compute: first set it to 0, sum the entire font as ULONG, then store
    0xB1B0AFBA - sum.

*/

#  include "font/luatexfont.h"
#  undef MIN
#  define MIN(a, b) (((a) < (b)) ? (a) : (b))
#  define STREAM_COMPRESS

static unsigned char wbuf[1024], padbytes[4] = { 0, 0, 0, 0 };

pdf_obj *sfnt_create_FontFile_stream(sfnt * sfont)
{
    pdf_obj *stream;
    struct sfnt_table_directory *td;
    long offset, nb_read, length;
    int i, sr;
    char *p;
    stream = pdf_new_stream(STREAM_COMPRESS);
    td = sfont->directory;
    p = (char *) wbuf;
    p += sfnt_put_ulong(p, (LONG) td->version);
    p += sfnt_put_ushort(p, td->num_kept_tables);
    sr = (int) (max2floor(td->num_kept_tables) * 16);
    p += sfnt_put_ushort(p, sr);
    p += sfnt_put_ushort(p, log2floor(td->num_kept_tables));
    p += sfnt_put_ushort(p, td->num_kept_tables * 16 - sr);
    pdf_add_stream(stream, wbuf, 12);
    /*tex Compute start of actual tables (after headers). */
    offset = 12 + 16 * td->num_kept_tables;
    for (i = 0; i < td->num_tables; i++) {
        /*tex This table must exist in |FontFile|: */
        if (td->flags[i] & SFNT_TABLE_REQUIRED) {
            if ((offset % 4) != 0) {
                offset += 4 - (offset % 4);
            }
            p = (char *) wbuf;
            memcpy(p, td->tables[i].tag, 4);
            p += 4;
            p += sfnt_put_ulong(p, (LONG) td->tables[i].check_sum);
            p += sfnt_put_ulong(p, offset);
            p += sfnt_put_ulong(p, (LONG) td->tables[i].length);
            pdf_add_stream(stream, wbuf, 16);
            offset = (long) (offset + (long) td->tables[i].length);
        }
    }
    offset = 12 + 16 * td->num_kept_tables;
    for (i = 0; i < td->num_tables; i++) {
        if (td->flags[i] & SFNT_TABLE_REQUIRED) {
            if ((offset % 4) != 0) {
                length = 4 - (offset % 4);
                pdf_add_stream(stream, padbytes, length);
                offset += length;
            }
            if (!td->tables[i].data) {
                if (!sfont->buffer) {
                    pdf_release_obj(stream);
                    normal_error("ttf","file not opened or already closed");
                    return NULL;
                }
                length = (long) td->tables[i].length;
                sfnt_seek_set(sfont, (long) td->tables[i].offset);
                while (length > 0) {
                    nb_read = sfnt_read(wbuf, (int) MIN(length, 1024), sfont);
                    if (nb_read < 0) {
                        pdf_release_obj(stream);
                        normal_error("ttf","reading file failed");
                        return NULL;
                    } else if (nb_read > 0) {
                        pdf_add_stream(stream, wbuf, nb_read);
                    }
                    length -= nb_read;
                }
            } else {
                pdf_add_stream(stream,
                               (unsigned char *) td->tables[i].data,
                               (long) td->tables[i].length);
                RELEASE(td->tables[i].data);
                td->tables[i].data = NULL;
            }
            /*tex Set offset for next table. */
            offset = (long) (offset + (long) td->tables[i].length);
        }
    }
    return stream;
}
