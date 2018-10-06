/* epdf.h

   Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
   Copyright 2006-2015 Taco Hoekwater <taco@luatex.org>
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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


/* this is the common header file for C++ sources pdftoepdf.c and lepdflib.c */

#ifndef EPDF_H
#  define EPDF_H

/*extern "C" {*/

#ifdef HAVE_CONFIG_H
#include <w2c/config.h>
#endif

/*}*/

#  include <stdlib.h>
#  include <math.h>
#  include <stddef.h>
#  include <stdio.h>
#  include <string.h>
#  include <kpathsea/c-ctype.h>
#  include <sys/stat.h>
#  include <dirent.h>

/*extern "C" { */

#  include <kpathsea/c-auto.h>

#if !defined(MIKTEX)
extern char *xstrdup(const char *);
#endif

typedef enum { FE_FAIL, FE_RETURN_NULL } file_error_mode;

/* the following code is extremly ugly but needed for including web2c/config.h */

#if !defined(MIKTEX)
typedef const char *const_string;   /* including kpathsea/types.h doesn't work on some systems */
#endif

#  define KPATHSEA_CONFIG_H     /* avoid including other kpathsea header files */

/* from web2c/config.h */

#  ifdef CONFIG_H               /* CONFIG_H has been defined by some xpdf */
#    undef CONFIG_H             /* header file */
#  endif

#  include <c-auto.h>           /* define SIZEOF_LONG */

#  include "image.h"
#  include "utils/avlstuff.h"
#  include "pdf/pdftypes.h"

#  include "lua.h"
#  include "lauxlib.h"

# include "luapplib/pplib.h"

/* pdfgen.w */

extern int ten_pow[10];
#if !(defined(MIKTEX) && defined(_MSC_VER))
__attribute__ ((format(printf, 2, 3)))
#endif
extern void pdf_printf(PDF, const char *fmt, ...);
extern void pdf_begin_obj(PDF, int, int);
extern void pdf_end_obj(PDF);
extern void pdf_begin_dict(PDF);
extern void pdf_end_dict(PDF);
extern void pdf_begin_array(PDF);
extern void pdf_end_array(PDF);
extern void pdf_add_null(PDF);
extern void pdf_add_bool(PDF, int i);
extern void pdf_add_int(PDF, int i);
extern void pdf_add_real(PDF, double d);
extern void pdf_add_ref(PDF, int num);
extern void pdf_add_name(PDF, const char *name);
extern void pdf_dict_add_streaminfo(PDF);
extern void pdf_begin_stream(PDF);
extern void pdf_end_stream(PDF);
extern void pdf_room(PDF, int);
extern void pdf_out_block(PDF pdf, const char *s, size_t n);

extern void pdf_dict_add_int(PDF, const char *key, int i);
extern void pdf_dict_add_ref(PDF, const char *key, int num);
extern void pdf_dict_add_name(PDF, const char *key, const char *val);
extern void pdf_dict_add_streaminfo(PDF);

/* Conflict with pdfgen.h */
/*#  define pdf_out(pdf, A) do { pdf_room(pdf, 1); *(pdf->buf->p++) = A; } while (0)*/
/*#  define pdf_quick_out(pdf,A) *(pdf->buf->p++)=(unsigned char)(A) */

#  define pdf_puts(pdf, s) pdf_out_block((pdf), (s), strlen(s))

/* pdfpage.w */

extern void print_pdffloat(PDF pdf, pdffloat f);

/* pdftables.w */

extern int pdf_create_obj(PDF pdf, int t, int i);

/* pdftoepdf.c */

extern void read_pdf_info(image_dict *);
extern void flush_pdf_info(image_dict *);

extern void write_epdf(PDF, image_dict *, int suppress_optional_info);
extern int  write_epdf_object(PDF, image_dict *, int n);

extern void unrefPdfDocument(char *);
extern void unrefMemStreamPdfDocument(char *);

extern void epdf_free(void);

/* writeimg.w */

extern void pdf_dict_add_img_filename(PDF pdf, image_dict * idict);

/* utils.w */

/*extern char *convertStringToPDFString(const char *in, int len);*/

/* lepdflib.w */

int luaopen_epdf(lua_State * L);

#  include "luatex-common.h"

/*}*/

typedef struct InObj InObj;

struct InObj {
    ppref *ref;                 /* ref in original PDF */
    int num;                    /* new object number in output PDF */
    InObj *next;                /* next entry in list of indirect objects */
} ;


typedef struct avl_table avl_table;

struct PdfDocument {
    char *file_path;            /* full file name including path */
    char *checksum;             /* for reopening */
    ppdoc *pdfe;
    InObj *inObjList;           /* temporary linked list */
    avl_table *ObjMapTree;      /* permanent over luatex run */
    int is_mem;
    char *memstream;
    unsigned int occurences;    /* number of references to the PdfDocument; it can be deleted when occurences == 0 */
    unsigned int pc;            /* counter to track PDFDoc generation or deletion */
};

typedef struct PdfDocument PdfDocument;

PdfDocument *refPdfDocument(const char *file_path, file_error_mode fe, const char *userpassword, const char *ownerpassword);

PdfDocument *refMemStreamPdfDocument(char *docstream, unsigned long long streamsize, const char *file_id);

#define STREAM_CHECKSUM_SIZE 16    // md5
#define STRSTREAM_CHECKSUM_SIZE 1+((unsigned int)(log((double)ULONG_MAX)/log(16.0)))    // djb2 printable digest as hex stream
#define STREAM_FILE_ID_LEN   2048  // 2048 bytes are enough to make a strong almost-unique name
#define STREAM_URI           "data:application/pdf,"
#define STREAM_URI_LEN       21    // length of "data:application/pdf," without final '\0'

#endif                          /* EPDF_H */
