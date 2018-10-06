/*

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2012 Taco Hoekwater <taco@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU General Public License along with
LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"

/*tex

    This is a trick to load mingw32's io.h early, using a macro redefinition of
    |eof()|.

*/

#include <kpathsea/config.h>
#if defined(MIKTEX_WINDOWS)
#  include <miktex/unxemu.h> /* snprintf */
#endif
#include "sys/types.h"
#include <kpathsea/c-stat.h>
#include <kpathsea/c-fopen.h>
#include <string.h>
#include <time.h>

/*tex For |DBL_EPSILON|: */

#include <float.h>

#include "zlib.h"
#include "md5.h"

#include "lua/luatex-api.h"
#include "luatex_svnversion.h"

#include "png.h"
#include "mplib.h"

#define check_nprintf(size_get, size_want) \
    if ((unsigned)(size_get) >= (unsigned)(size_want)) \
        formatted_error("internal","snprintf failed: file %s, line %d", __FILE__, __LINE__);

char *cur_file_name = NULL;
static char print_buf[PRINTF_BUF_SIZE];
int epochseconds;
int microseconds;

typedef char char_entry;
define_array(char);

#define SUBSET_TAG_LENGTH 6

void make_subset_tag(fd_entry * fd)
{
    int i, j = 0, a[SUBSET_TAG_LENGTH];
    md5_state_t pms;
    char *glyph;
    glw_entry *glw_glyph;
    struct avl_traverser t;
    md5_byte_t digest[16];
    void **aa;
    static struct avl_table *st_tree = NULL;
    if (st_tree == NULL)
        st_tree = avl_create(comp_string_entry, NULL, &avl_xallocator);
    assert(fd != NULL);
    assert(fd->gl_tree != NULL);
    assert(fd->fontname != NULL);
    assert(fd->subset_tag == NULL);
    fd->subset_tag = xtalloc(SUBSET_TAG_LENGTH + 1, char);
    do {
        md5_init(&pms);
        avl_t_init(&t, fd->gl_tree);
        if (is_cidkeyed(fd->fm)) {      /* |glw_entry| items */
            for (glw_glyph = (glw_entry *) avl_t_first(&t, fd->gl_tree);
                 glw_glyph != NULL; glw_glyph = (glw_entry *) avl_t_next(&t)) {
                glyph = malloc(24);
                sprintf(glyph, "%05u%05u ", glw_glyph->id, glw_glyph->wd);
                md5_append(&pms, (md5_byte_t *) glyph, (int) strlen(glyph));
                free(glyph);
            }
        } else {
            for (glyph = (char *) avl_t_first(&t, fd->gl_tree); glyph != NULL;
                 glyph = (char *) avl_t_next(&t)) {
                md5_append(&pms, (md5_byte_t *) glyph, (int) strlen(glyph));
                md5_append(&pms, (const md5_byte_t *) " ", 1);
            }
        }
        md5_append(&pms, (md5_byte_t *) fd->fontname,
                   (int) strlen(fd->fontname));
        md5_append(&pms, (md5_byte_t *) & j, sizeof(int));      /* to resolve collision */
        md5_finish(&pms, digest);
        for (a[0] = 0, i = 0; i < 13; i++)
            a[0] += digest[i];
        for (i = 1; i < SUBSET_TAG_LENGTH; i++)
            a[i] = a[i - 1] - digest[i - 1] + digest[(i + 12) % 16];
        for (i = 0; i < SUBSET_TAG_LENGTH; i++)
            fd->subset_tag[i] = (char) (a[i] % 26 + 'A');
        fd->subset_tag[SUBSET_TAG_LENGTH] = '\0';
        j++;
        assert(j < 100);
    }
    while ((char *) avl_find(st_tree, fd->subset_tag) != NULL);
    aa = avl_probe(st_tree, fd->subset_tag);
    assert(aa != NULL);
    if (j > 2)
        formatted_warning("subsets","subset-tag collision, resolved in round %d",j);
}

__attribute__ ((format(printf, 1, 2)))
void tex_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(print_buf, PRINTF_BUF_SIZE, fmt, args);
    tprint(print_buf);
    xfflush(stdout);
    va_end(args);
}

size_t xfwrite(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
    if (fwrite(ptr, size, nmemb, stream) != nmemb)
        formatted_error("file io","fwrite() failed");
    return nmemb;
}

int xfflush(FILE * stream)
{
    if (fflush(stream) != 0)
        formatted_error("file io","fflush() failed (%s)", strerror(errno));
    return 0;
}

int xgetc(FILE * stream)
{
    int c = getc(stream);
    if (c < 0 && c != EOF)
        formatted_error("file io","getc() failed (%s)", strerror(errno));
    return c;
}

int xputc(int c, FILE * stream)
{
    int i = putc(c, stream);
    if (i < 0)
        formatted_error("file io","putc() failed (%s)", strerror(errno));
    return i;
}

scaled ext_xn_over_d(scaled x, scaled n, scaled d)
{
    double r = (((double) x) * ((double) n)) / ((double) d);
    if (r > DBL_EPSILON)
        r += 0.5;
    else
        r -= 0.5;
    if (r >= (double) max_integer || r <= -(double) max_integer)
        normal_warning("internal","arithmetic number too big");
    return (scaled) r;
}

/*tex

    This function strips trailing zeros in string with numbers; leading zeros are
    not stripped (as in real life), It's not used.

*/

#if 0
char *stripzeros(char *a)
{
    enum { NONUM, DOTNONUM, INT, DOT, LEADDOT, FRAC } s = NONUM, t = NONUM;
    char *p, *q, *r;
    for (p = q = r = a; *p != '\0';) {
        switch (s) {
        case NONUM:
            if (*p >= '0' && *p <= '9')
                s = INT;
            else if (*p == '.')
                s = LEADDOT;
            break;
        case DOTNONUM:
            if (*p != '.' && (*p < '0' || *p > '9'))
                s = NONUM;
            break;
        case INT:
            if (*p == '.')
                s = DOT;
            else if (*p < '0' || *p > '9')
                s = NONUM;
            break;
        case DOT:
        case LEADDOT:
            if (*p >= '0' && *p <= '9')
                s = FRAC;
            else if (*p == '.')
                s = DOTNONUM;
            else
                s = NONUM;
            break;
        case FRAC:
            if (*p == '.')
                s = DOTNONUM;
            else if (*p < '0' || *p > '9')
                s = NONUM;
            break;
        default:;
        }
        switch (s) {
        case DOT:
            r = q;
            break;
        case LEADDOT:
            r = q + 1;
            break;
        case FRAC:
            if (*p > '0')
                r = q + 1;
            break;
        case NONUM:
            if ((t == FRAC || t == DOT) && r != a) {
                q = r--;
                if (*r == '.')  /* was a LEADDOT */
                    *r = '0';
                r = a;
            }
            break;
        default:;
        }
        *q++ = *p++;
        t = s;
    }
    *q = '\0';
    return a;
}
#endif

void initversionstring(char **versions)
{

#ifdef LuajitTeX
#define LUA_VER_STRING  LUAJIT_VERSION
#else
#define LUA_VER_STRING  "lua version " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE
#endif
#define STR(tok) STR2(tok)
#define STR2(tok) #tok

    const_string fmt =
        "Compiled with libpng %s; using %s\n"
        "Compiled with %s\n" /* Lua or LuaJIT */
        "Compiled with mplib version %s\n"
        "Compiled with zlib %s; using %s\n"
        "\nDevelopment id: %s\n";
    size_t len = strlen(fmt)
               + strlen(PNG_LIBPNG_VER_STRING) + strlen(png_libpng_ver)
               + strlen(LUA_VER_STRING)
               + strlen(mp_metapost_version())
               + strlen(ZLIB_VERSION) + strlen(zlib_version)
               + strlen(STR(luatex_svn_revision))
               + 1;

    /*tex
        The size of |len| will be more than enough, because of the placeholder
        chars in fmt that get replaced by the arguments.
    */
    *versions = xmalloc(len);
    sprintf(*versions, fmt,
                    PNG_LIBPNG_VER_STRING, png_libpng_ver, LUA_VER_STRING,
                    mp_metapost_version(),
                    ZLIB_VERSION, zlib_version,STR(luatex_svn_revision));

#undef STR2
#undef STR
#undef LUA_VER_STRING

}

void check_buffer_overflow(int wsize)
{
    if (wsize > buf_size) {
        int nsize = buf_size + buf_size / 5 + 5;
        if (nsize < wsize) {
            nsize = wsize + 5;
        }
        buffer = (unsigned char *) xreallocarray(buffer, char, (unsigned) nsize);
        buf_size = nsize;
    }
}

/*tex

    The return value is a decimal number with the point |dd| places from the
    back, |scaled_out| is the number of scaled points corresponding to that.

*/

#define max_integer 0x7FFFFFFF

scaled divide_scaled(scaled s, scaled m, int dd)
{
    register scaled q;
    register scaled r;
    int i;
    int sign = 1;
    if (s < 0) {
        sign = -sign;
        s = -s;
    }
    if (m < 0) {
        sign = -sign;
        m = -m;
    }
    if (m == 0) {
        normal_error("arithmetic", "divided by zero");
    } else if (m >= (max_integer / 10)) {
        normal_error("arithmetic", "number too big");
    }
    q = s / m;
    r = s % m;
    for (i = 1; i <= (int) dd; i++) {
        q = 10 * q + (10 * r) / m;
        r = (10 * r) % m;
    }
    /*tex Rounding: */
    if (2 * r >= m) {
        q++;
    }
    return sign * q;
}

#if !defined(MIKTEX)
#ifdef _WIN32
#undef floor
#define floor win32_floor
#endif
#endif

/*tex

    The same function, but using doubles instead of integers (faster).

*/

scaled divide_scaled_n(double sd, double md, double n)
{
    double dd, di = 0.0;
    dd = sd / md * n;
    if (dd > 0.0)
        di = floor(dd + 0.5);
    else if (dd < 0.0)
        di = -floor((-dd) + 0.5);
    return (scaled) di;
}

int do_zround(double r)
{
    int i;
    if (r > 2147483647.0)
        i = 2147483647;
    else if (r < -2147483647.0)
        i = -2147483647;
    else if (r >= 0.0)
        i = (int) (r + 0.5);
    else
        i = (int) (r - 0.5);
    return i;
}


/*tex

    Old MSVC doesn't have |rint|.

*/

#if defined(_MSC_VER) && _MSC_VER <= 1600

#  include <math.h>

double rint(double x)
{
    return floor(x+0.5);
}

#endif

/*tex

    We replace |tmpfile| on \MSWINDOWS:

*/

#if defined(_WIN32)

/*

    _cairo_win_tmpfile (void) - replace tmpfile() on Windows
    extracted from cairo-misc.c in cairo - a vector graphics library
    with display and print output

    the functiion name is changed from _cairo_win32_tmpfile (void) to
    _cairo_win_tmpfile (void)

    Copyright 2002 University of Southern California
    Copyright 2005 Red Hat, Inc.
    Copyright 2007 Adrian Johnson

    This library is free software; you can redistribute it and/or modify it
    either under the terms of the GNU Lesser General Public License version 2.1
    as published by the Free Software Foundation (the "LGPL") or, at your option,
    under the terms of the Mozilla Public License Version 1.1 (the "MPL"). If you
    do not alter this notice, a recipient may use your version of this file under
    either the MPL or the LGPL.

    You should have received a copy of the LGPL along with this library in the
    file COPYING-LGPL-2.1; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA You should have
    received a copy of the MPL along with this library in the file
    COPYING-MPL-1.1

    The contents of this file are subject to the Mozilla Public License Version
    1.1 (the "License"); you may not use this file except in compliance with the
    License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

    This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
    KIND, either express or implied. See the LGPL or the MPL for the specific
    language governing rights and limitations.

    The Original Code is the cairo graphics library. The Initial Developer of the
    Original Code is University of Southern California. Contributor(s):

    Carl D. Worth  <cworth@cworth.org>
    Adrian Johnson <ajohnson@redneon.com>

*/

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN

/*tex

    We require \MSWINDOWS\ 2000 features such as |ETO_PDY|. We probably can now
    assume that all \MSWINDOWS\ versions are recent.

*/

#if !defined(WINVER) || (WINVER < 0x0500)
# define WINVER 0x0500
#endif
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
# define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <io.h>

/*tex

    On \MSWINDOWS\ |tmpfile| creates the file in the root directory. This may
    fail due to unsufficient privileges. However, this isn't a problem on
    \MSWINDOWS\ CE so we don't use it there. Who is actually using CE anyway?

*/

FILE * _cairo_win_tmpfile (void)
{
    DWORD path_len;
    WCHAR path_name[MAX_PATH + 1];
    WCHAR file_name[MAX_PATH + 1];
    HANDLE handle;
    int fd;
    FILE *fp;
    path_len = GetTempPathW (MAX_PATH, path_name);
    if (path_len <= 0 || path_len >= MAX_PATH)
        return NULL;
    if (GetTempFileNameW (path_name, L"ps_", 0, file_name) == 0)
        return NULL;
    handle = CreateFileW (file_name,
			 GENERIC_READ | GENERIC_WRITE,
			 0,
			 NULL,
			 CREATE_ALWAYS,
			 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
			 NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        DeleteFileW (file_name);
        return NULL;
    }
    fd = _open_osfhandle((intptr_t) handle, 0);
    if (fd < 0) {
        CloseHandle (handle);
        return NULL;
    }
    fp = _fdopen(fd, "w+b");
    if (fp == NULL) {
        _close(fd);
        return NULL;
    }
    return fp;
}

#endif
