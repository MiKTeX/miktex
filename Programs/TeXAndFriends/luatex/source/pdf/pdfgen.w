% pdfgen.w
%
% Copyright 2009-2013 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c
#include "ptexlib.h"

@ @c
#include <kpathsea/c-dir.h>
#include <kpathsea/c-ctype.h>
#include "lua/luatex-api.h"
#include "md5.h"

#define check_nprintf(size_get, size_want) \
    if ((unsigned)(size_get) >= (unsigned)(size_want)) \
        formatted_error("pdf backend","snprintf() failed in file %s at line %d", __FILE__, __LINE__);

PDF static_pdf = NULL;

@ commandline interface

@c
int output_mode_used;
int output_mode_option;
int output_mode_value;
int draft_mode_option;
int draft_mode_value;

halfword pdf_info_toks;                              /* additional keys of Info dictionary */
halfword pdf_catalog_toks;                           /* additional keys of Catalog dictionary */
halfword pdf_catalog_openaction;
halfword pdf_names_toks;                             /* additional keys of Names dictionary */
halfword pdf_trailer_toks;                           /* additional keys of Trailer dictionary */
shipping_mode_e global_shipping_mode = NOT_SHIPPING; /* set to |shipping_mode| when |ship_out| starts */


@ Create a new buffer |strbuf_s| of size |size| and maximum allowed size |limit|.
Initialize it and set |p| to begin of data.

@c
strbuf_s *new_strbuf(size_t size, size_t limit)
{
    strbuf_s *b;
    b = xtalloc(1, strbuf_s);
    b->size = size;
    b->limit = limit;
    if (size > 0)
        b->p = b->data = xtalloc(b->size, unsigned char);
    else
        b->p = b->data = NULL; /* for other alloc */
    return b;
}

@ Check that |n| bytes more fit into buffer; increase it if required.

@c
static void strbuf_room(strbuf_s * b, size_t n)
{
    unsigned int a;
    size_t l = (size_t) (b->p - b->data);
    if (n > b->limit - l)
        overflow("PDF buffer", (unsigned) b->size);
    if (n + l > b->size) {
        a = (unsigned int) (b->size >> 2);
        if (n + l > b->size + a)
            b->size = n + l;
        else if (b->size < b->limit - a)
            b->size = b->size + a;
        else
            b->size = b->limit;
        b->data = xreallocarray(b->data, unsigned char, (unsigned) b->size);
        b->p = b->data + l;
    }
}

@ Seek to position |offset| within buffer. Position must be valid.

@c
void strbuf_seek(strbuf_s * b, off_t offset)
{
    b->p = b->data + offset;
}

@ Get the current buffer fill level, the number of characters.

@c
size_t strbuf_offset(strbuf_s * b)
{
    return (size_t) (b->p - b->data);
}

@ Put one character into buffer. Make room before if needed.

@c
void strbuf_putchar(strbuf_s * b, unsigned char c)
{
    if ((size_t) (b->p - b->data + 1) > b->size)
        strbuf_room(b, 1);
    *b->p++ = c;
}

@ Dump filled buffer part to PDF.

@c
void strbuf_flush(PDF pdf, strbuf_s * b)
{
    pdf_out_block(pdf, (const char *) b->data, strbuf_offset(b));
    strbuf_seek(b, 0);
}

@ Free all dynamically allocated buffer structures.

@c
void strbuf_free(strbuf_s * b)
{
    xfree(b->data);
    xfree(b);
}

@ |init_pdf_struct()| is called early, only once, from maincontrol.w

@c
PDF init_pdf_struct(PDF pdf)
{
    os_struct *os;
    pdf = xtalloc(1, pdf_output_file);
    memset(pdf, 0, sizeof(pdf_output_file));
    pdf->job_name = makecstring(job_name);

    output_mode_used = OMODE_NONE;  /* will be set by |fix_o_mode()| */
    pdf->o_mode = output_mode_used; /* used by synctex, we need to use output_mode_used there */
    pdf->o_state = ST_INITIAL;

    /* init PDF and object stream writing */
    pdf->os = os = xtalloc(1, os_struct);
    memset(pdf->os, 0, sizeof(os_struct));
    os->buf[PDFOUT_BUF] = new_strbuf(inf_pdfout_buf_size, sup_pdfout_buf_size);
    os->buf[OBJSTM_BUF] = new_strbuf(inf_objstm_buf_size, sup_objstm_buf_size);
    os->obj = xtalloc(PDF_OS_MAX_OBJS, os_obj_data);
    os->cur_objstm = 0;

    os->curbuf = PDFOUT_BUF;
    pdf->buf = os->buf[os->curbuf];
    /*
        Later  ttf_seek_outbuf(TABDIR_OFF + n * 4 * TTF_ULONG_SIZE); in ttf_init_font
        we need 236 bytes, so we start with 256 bytes as in pdftex.
    */
    pdf->fb = new_strbuf(256, 100000000);

    pdf->stream_deflate = false;
    pdf->stream_writing = false;

    /*
        Sometimes it is neccesary to allocate memory for PDF output that cannot be
        deallocated then, so we use |mem| for this purpose.
    */
    pdf->mem_size = inf_pdf_mem_size;
    pdf->mem = xtalloc(pdf->mem_size, int);
    /*
        The first word is not used so we can use zero as a value for testing
        whether a pointer to |mem| is valid.
    */
    pdf->mem_ptr = 1;

    pdf->pstruct = NULL;

    pdf->posstruct = xtalloc(1, posstructure);
    pdf->posstruct->pos.h = 0;
    pdf->posstruct->pos.v = 0;
    pdf->posstruct->dir = dir_TLT;

    /* allocated size of |obj_tab| array */
    pdf->obj_tab_size = (unsigned) inf_obj_tab_size;
    pdf->obj_tab = xtalloc(pdf->obj_tab_size + 1, obj_entry);
    memset(pdf->obj_tab, 0, sizeof(obj_entry));

    pdf->minor_version = -1;
    pdf->decimal_digits = 4;
    pdf->gamma = 65536;
    pdf->image_gamma = 65536;
    pdf->image_hicolor = 1;
    pdf->image_apply_gamma = 0;
    pdf->objcompresslevel = 0;
    pdf->compress_level = 0;
    pdf->draftmode = 0;
    pdf->inclusion_copy_font = 1;
    pdf->pk_resolution = 0;
    pdf->pk_fixed_dpi = 0;
    pdf->pk_scale_factor = 0;

    init_dest_names(pdf);
    pdf->page_resources = NULL;

    init_pdf_pagecalculations(pdf);

    pdf->vfstruct = new_vfstruct();

    return pdf;
}

@  We use |pdf_get_mem| to allocate memory in |mem|.

@c
int pdf_get_mem(PDF pdf, int s)
{
    int a;
    int ret;
    if (s > sup_pdf_mem_size - pdf->mem_ptr)
        overflow("pdf memory size (pdf_mem_size)", (unsigned) pdf->mem_size);
    if (pdf->mem_ptr + s > pdf->mem_size) {
        a = pdf->mem_size >> 2;
        if (pdf->mem_ptr + s > pdf->mem_size + a) {
            pdf->mem_size = pdf->mem_ptr + s;
        } else if (pdf->mem_size < sup_pdf_mem_size - a) {
            pdf->mem_size = pdf->mem_size + a;
        } else {
            pdf->mem_size = sup_pdf_mem_size;
        }
        pdf->mem = xreallocarray(pdf->mem, int, (unsigned) pdf->mem_size);
    }
    ret = pdf->mem_ptr;
    pdf->mem_ptr = pdf->mem_ptr + s;
    return ret;
}

@ there are defined in |luatex-api.h|, so we need |luatex-api.c|:

@c
output_mode get_o_mode(void)
{
    output_mode o_mode;
    if (output_mode_par > 0) {
        o_mode = OMODE_PDF;
    } else
        o_mode = OMODE_DVI;
    return o_mode;
}

void fix_o_mode(void)
{
    output_mode o_mode = get_o_mode();
    if (output_mode_used == OMODE_NONE) {
        output_mode_used = o_mode;
        static_pdf->o_mode = output_mode_used; /* used by synctex, we need to use output_mode_used there */
    } else if (output_mode_used != o_mode) {
        normal_error("pdf backend", "\\outputmode can only be changed before anything is written to the output");
    }
}

@ This ensures that |pdfminorversion| is set only before any bytes have been
written to the generated \.{PDF} file. Here also all variables for \.{PDF} output
are initialized, the \.{PDF} file is opened by |ensure_pdf_open|, and the \.{PDF}
header is written.

@c
void fix_pdf_minorversion(PDF pdf)
{
    if (pdf->minor_version < 0) {       /* unset */
        if ((pdf_minor_version < 0) || (pdf_minor_version > 9)) {
            const char *hlp[] = { "The pdfminorversion must be between 0 and 9.", "I changed this to 4.", NULL };
            char msg[256];
            (void) snprintf(msg, 255, "LuaTeX error (illegal pdfminorversion %d)", (int) pdf_minor_version);
            tex_error(msg, hlp);
            pdf->minor_version = 4;
        } else {
            pdf->minor_version = pdf_minor_version;
        }
    } else {
        /* Check that variables for \.{PDF} output are unchanged */
        if (pdf->minor_version != pdf_minor_version)
            normal_error("pdf backend", "minorversion cannot be changed after data is written to the PDF file");
    }
}

static void fix_pdf_draftmode(PDF pdf)
{
    if (pdf->draftmode != draft_mode_par)
        normal_error("pdf backend", "draftmode cannot be changed after data is written to the PDF file");
    if (pdf->draftmode != 0) {
        pdf->compress_level = 0;        /* re-fix it, might have been changed inbetween */
        pdf->objcompresslevel = 0;
    }
}

@ @c
#define ZIP_BUF_SIZE  32768

#define check_err(f, fn) \
    if (f != Z_OK) \
        formatted_error("pdf backend","zlib %s() failed (error code %d)", fn, f)

@ @c
static void write_zip(PDF pdf)
{
    int flush, err = Z_OK;
    uInt zip_len;
    strbuf_s *buf = pdf->buf;
    z_stream *s = pdf->c_stream;
    boolean finish = pdf->zip_write_state == ZIP_FINISH;
    if (pdf->stream_length == 0) {
        if (s == NULL) {
            s = pdf->c_stream = xtalloc(1, z_stream);
            s->zalloc = (alloc_func) 0;
            s->zfree = (free_func) 0;
            s->opaque = (voidpf) 0;
            check_err(deflateInit(s, pdf->compress_level), "deflateInit");
            pdf->zipbuf = xtalloc(ZIP_BUF_SIZE, char);
        } else
            check_err(deflateReset(s), "deflateReset");
        s->next_out = (Bytef *) pdf->zipbuf;
        s->avail_out = ZIP_BUF_SIZE;
    }
    s->next_in = buf->data;
    s->avail_in = (uInt) (buf->p - buf->data);
    while (true) {
        if (s->avail_out == 0 || (finish && s->avail_out < ZIP_BUF_SIZE)) {
            zip_len = ZIP_BUF_SIZE - s->avail_out;
            pdf->gone += (off_t) xfwrite(pdf->zipbuf, 1, zip_len, pdf->file);
            pdf->last_byte = pdf->zipbuf[zip_len - 1];
            s->next_out = (Bytef *) pdf->zipbuf;
            s->avail_out = ZIP_BUF_SIZE;
        }
        if (finish) {
            if (err == Z_STREAM_END) {
                xfflush(pdf->file);
                pdf->zip_write_state = NO_ZIP;
                break;
            }
            flush = Z_FINISH;
        } else {
            if (s->avail_in == 0)
                break;
            flush = Z_NO_FLUSH;
        }
        err = deflate(s, flush);
        if (err != Z_OK && err != Z_STREAM_END)
            formatted_error("pdf backend","zlib deflate() failed (error code %d)", err);
    }
    pdf->stream_length = (off_t) s->total_out;
}

@ @c
void zip_free(PDF pdf)
{
    if (pdf->zipbuf != NULL) {
        check_err(deflateEnd(pdf->c_stream), "deflateEnd");
        xfree(pdf->zipbuf);
    }
    xfree(pdf->c_stream);
}

@ @c
static void write_nozip(PDF pdf)
{
    strbuf_s *buf = pdf->buf;
    size_t l = strbuf_offset(buf);
    if (l == 0)
        return;
    pdf->stream_length = pdf_offset(pdf) - pdf->save_offset;
    pdf->gone += (off_t) xfwrite((char *) buf->data, sizeof(char), l, pdf->file);
    pdf->last_byte = *(buf->p - 1);
}

@ The PDF buffer is flushed by calling |pdf_flush|, which checks the
variable |zip_write_state| and will compress the buffer before flushing if
neccesary. We call |pdf_begin_stream| to begin a stream  and |pdf_end_stream|
to finish it. The stream contents will be compressed if compression is turn on.

@c
void pdf_flush(PDF pdf)
{
    os_struct *os = pdf->os;
    off_t saved_pdf_gone = pdf->gone;
    switch (os->curbuf) {
        case PDFOUT_BUF:
            if (pdf->draftmode == 0) {
                switch (pdf->zip_write_state) {
                    case NO_ZIP:
                        write_nozip(pdf);
                        break;
                    case ZIP_WRITING:
                    case ZIP_FINISH:
                        write_zip(pdf);
                        break;
                    default:
                        normal_error("pdf backend", "bad zip state");
                }
            } else
                pdf->zip_write_state = NO_ZIP;
            strbuf_seek(pdf->buf, 0);
            if (saved_pdf_gone > pdf->gone)
                normal_error("pdf backend", "file size exceeds architectural limits (pdf_gone wraps around)");
            break;
        case OBJSTM_BUF:
            break;
        default:
            normal_error("pdf backend", "bad buffer state");
    }
}

@ @c
static void pdf_buffer_select(PDF pdf, buffer_e buf)
{
    os_struct *os = pdf->os;
    if (pdf->os_enable && buf == OBJSTM_BUF)
        os->curbuf = OBJSTM_BUF; /* switch to object stream */
    else
        os->curbuf = PDFOUT_BUF; /* switch to PDF stream */
    pdf->buf = os->buf[pdf->os->curbuf];
}

@ create new \.{/ObjStm} object if required, and set up cross reference info

@c
static void pdf_prepare_obj(PDF pdf, int k, int pdf_os_threshold)
{
    os_struct *os = pdf->os;
    strbuf_s *obuf = os->buf[OBJSTM_BUF];
    if (pdf->objcompresslevel >= pdf_os_threshold)
        pdf_buffer_select(pdf, OBJSTM_BUF);
    else
        pdf_buffer_select(pdf, PDFOUT_BUF);
    switch (os->curbuf) {
        case PDFOUT_BUF:
            obj_offset(pdf, k) = pdf_offset(pdf);
            obj_os_idx(pdf, k) = PDF_OS_MAX_OBJS;   /* mark it as not included in any ObjStm */
            break;
        case OBJSTM_BUF:
            if (os->cur_objstm == 0) {
                os->cur_objstm =
                    (unsigned int) pdf_create_obj(pdf, obj_type_objstm, 0);
                os->idx = 0;
                obuf->p = obuf->data;       /* start fresh object stream */
                os->ostm_ctr++;     /* only for statistics */
            }
            obj_os_idx(pdf, k) = (int) os->idx;
            obj_os_objnum(pdf, k) = (int) os->cur_objstm;
            os->obj[os->idx].num = k;
            os->obj[os->idx].off = obuf->p - obuf->data;
            break;
        default:
            normal_error("pdf backend", "bad object state");
    }
}

@* Low-level buffer checkers.

@ Set the active buffer pointer. Make sure that there are at least |n| bytes free
in that buffer, flush if needed.

@c
inline void pdf_room(PDF pdf, int n)
{
    strbuf_s *buf = pdf->buf;
    if ((size_t) (n + buf->p - buf->data) <= buf->size)
        return;
    if (pdf->os->curbuf == PDFOUT_BUF) {
        if ((size_t) n > buf->size)
            overflow("PDF output buffer", (unsigned) buf->size);
        if ((size_t) (n + buf->p - buf->data) < buf->limit)
            strbuf_room(buf, (size_t) n);
        else
            pdf_flush(pdf);
    } else {
        strbuf_room(buf, (size_t) n);
    }
}

@ @c
void pdf_out_block(PDF pdf, const char *s, size_t n)
{
    size_t l;
    strbuf_s *buf = pdf->buf;
    do {
        l = n;
        if (l > buf->size)
            l = buf->size;
        pdf_room(pdf, (int) l);
        (void) memcpy(buf->p, s, l);
        buf->p += l;
        s += l;
        n -= l;
    } while (n > 0);
}

@ @c
__attribute__ ((format(printf, 2, 3)))
void pdf_printf(PDF pdf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (pdf->printf_buf == NULL) {
        pdf->printf_buf = xtalloc(PRINTF_BUF_SIZE, char);
    }
    (void) vsnprintf(pdf->printf_buf, PRINTF_BUF_SIZE, fmt, args);
    pdf_puts(pdf, pdf->printf_buf);
    va_end(args);
}

@ print out a string to PDF buffer

@c
void pdf_print(PDF pdf, str_number s)
{
    const char *ss;
    size_t l;
    if (s >= STRING_OFFSET) {
        ss = (const char *) str_string(s);
        l = str_length(s);
        pdf_out_block(pdf, ss, l);
    } else {
        pdf_out(pdf, s);
    }
}

@ print out a integer to PDF buffer

@c
void pdf_print_int(PDF pdf, longinteger n)
{
    char s[24];
    int w;
    w = snprintf(s, 23, "%" LONGINTEGER_PRI "d", (LONGINTEGER_TYPE) n);
    check_nprintf(w, 23);
    pdf_out_block(pdf, (const char *) s, (size_t) w);
}

@ @c
/*
void print_pdffloat(PDF pdf, pdffloat f)
{
    char a[24];
    int e = f.e, i, l;
    int64_t m = f.m;
    if (m < 0) {
        pdf_out(pdf, '-');
        m *= -1;
    }
    l = m / ten_pow[e];
    pdf_print_int(pdf, l);
    l = m % ten_pow[e];
    if (l != 0) {
        pdf_out(pdf, '.');
        snprintf(a, 23, "%d", l + ten_pow[e]);
        for (i = e; i > 0; i--) {
            if (a[i] != '0')
                break;
            a[i] = '\0';
        }
        pdf_puts(pdf, (a + 1));
    }
}
*/

void print_pdffloat(PDF pdf, pdffloat f)
{
    int64_t m = f.m;
    if (m == 0) {
        pdf_out(pdf, '0');
    } else {
        int e = f.e;
        if (e == 0) {
            /* division by ten_pow[0] == 1 */
            if (m == 1) {
                pdf_out(pdf, '1');
            } else {
                char a[24];
                snprintf(a, 23, "%" LONGINTEGER_PRI "i", (LONGINTEGER_TYPE) m);
                pdf_puts(pdf, a);
            }
        } else {
            int t = ten_pow[e] ;
            if (t == m) {
                pdf_out(pdf, '1');
            } else {
                int i, l, w;
                char a[24];
                if (m < 0) {
                    pdf_out(pdf, '-');
                    m *= -1;
                }
                l = m / t;
                w = snprintf(a, 23, "%i", l);
                pdf_out_block(pdf, (const char *) a, (size_t) w);
                l = m % t;
                if (l != 0) {
                    pdf_out(pdf, '.');
                    snprintf(a, 23, "%d", l + t);
                    for (i = e; i > 0; i--) {
                        if (a[i] != '0')
                            break;
                        a[i] = '\0';
                    }
                    pdf_puts(pdf, (a + 1));
                }
            }
        }
    }
}

@ print out |s| as string in PDF output

@c
void pdf_print_str(PDF pdf, const char *s)
{
    const char *orig = s;
    int l = (int) strlen(s) - 1; /* last string index */
    if (l < 0) {
        pdf_puts(pdf, "()");
        return;
    }
    /* the next is not really safe, the string could be "(a)xx(b)" */
    if ((s[0] == '(') && (s[l] == ')')) {
        pdf_puts(pdf, s);
        return;
    }
    if ((s[0] != '<') || (s[l] != '>') || odd((l + 1))) {
        pdf_out(pdf, '(');
        pdf_puts(pdf, s);
        pdf_out(pdf, ')');
        return;
    }
    s++;
    while (isxdigit((unsigned char)*s))
        s++;
    if (s != orig + l) {
        pdf_out(pdf, '(');
        pdf_puts(pdf, orig);
        pdf_out(pdf, ')');
        return;
    }
    pdf_puts(pdf, orig); /* it was a hex string after all  */
}

@ begin a stream (needs to have a stream dictionary also)

@c
void pdf_begin_stream(PDF pdf)
{
    pdf_puts(pdf, "\nstream\n");
    pdf_save_offset(pdf);
    pdf_flush(pdf);
    if (pdf->stream_deflate) {
        pdf->zip_write_state = ZIP_WRITING;
    }
    pdf->stream_writing = true;
    pdf->stream_length = 0;
    pdf->last_byte = 0;
}

@ end a stream

@c
void pdf_end_stream(PDF pdf)
{
    os_struct *os = pdf->os;
    switch (os->curbuf) {
        case PDFOUT_BUF:
            if (pdf->zip_write_state == ZIP_WRITING)
                pdf->zip_write_state = ZIP_FINISH;
            pdf_flush(pdf);         /* sets pdf->last_byte */
            break;
        case OBJSTM_BUF:
            normal_error("pdf backend", "bad buffer in end stream, case 1");
            break;
        default:
            normal_error("pdf backend", "bad buffer in end stream, case 2");
    }
    pdf->stream_deflate = false;
    pdf->stream_writing = false;
    pdf_out(pdf, '\n');     /* doesn't really belong to the stream */
    pdf_puts(pdf, "endstream");
    /* write stream /Length */
    if (pdf->seek_write_length && pdf->draftmode == 0) {
        xfseeko(pdf->file, (off_t)pdf->stream_length_offset, SEEK_SET, pdf->job_name);
        fprintf(pdf->file, "%" LONGINTEGER_PRI "i", (LONGINTEGER_TYPE) pdf->stream_length);
        xfseeko(pdf->file, 0, SEEK_END, pdf->job_name);
    }
    pdf->seek_write_length = false;
}

@ To print |scaled| value to PDF output we need some subroutines to ensure
accurary.

@c
#define max_integer 0x7FFFFFFF  /* $2^{31}-1$ */

scaled one_hundred_inch =  7227 * 65536;             /* scaled value corresponds to 100in, exact, 473628672 */
scaled one_inch         = (7227 * 65536 + 50) / 100; /* scaled value corresponds to 1in (rounded to 4736287) */
scaled one_true_inch    = (7227 * 65536 + 50) / 100; /* scaled value corresponds to 1truein (rounded!) */
scaled one_hundred_bp   = (7227 * 65536)      /  72; /* scaled value corresponds to 100bp */
scaled one_bp           = 65781;                     /* scaled value corresponds to 1bp (rounded to 65782) */

/*
    one_bp is changed on 20110411 to be exactly 65781, as in tex itself, because this value
    is also used for \pdfpxdimen
*/

int ten_pow[10] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 /* $10^0..10^9$ */
};

@ The function |divide_scaled| divides |s| by |m| using |dd| decimal
digits of precision. It is defined in C because it is a good candidate
for optimizations that are not possible in pascal.

@c
scaled round_xn_over_d(scaled x, int n, unsigned int d)
{
    boolean positive = true;
    unsigned t, u, v;
    if (x < 0) {
        positive = !positive;
        x = -(x);
    }
    if (n < 0) {
        positive = !positive;
        n = -(n);
    }
    t = (unsigned) ((x % 0100000) * n);
    u = (unsigned) (((unsigned) (x) / 0100000) * (unsigned) n + (t / 0100000));
    v = (u % d) * 0100000 + (t % 0100000);
    if (u / d >= 0100000)
        arith_error = true;
    else
        u = 0100000 * (u / d) + (v / d);
    v = v % d;
    if (2 * v >= d)
        u++;
    if (positive)
        return (scaled) u;
    else
        return (-(scaled) u);
}

@ @c
void pdf_add_bp(PDF pdf, scaled s)
{
    pdffloat a;
    pdfstructure *p = pdf->pstruct;
    a.m = i64round(s * p->k1);
    a.e = pdf->decimal_digits;
    if (pdf->cave > 0)
        pdf_out(pdf, ' ');
    print_pdffloat(pdf, a);
    pdf->cave = 1;
}

@* handling page resources.

@c
typedef struct {
    int obj_type;
    pdf_object_list *list;
} pr_entry;

@ @c
static int comp_page_resources(const void *pa, const void *pb, void *p)
{
    int a = ((const pr_entry *) pa)->obj_type;
    int b = ((const pr_entry *) pb)->obj_type;
    (void) p;
    if (a > b)
        return 1;
    if (a < b)
        return -1;
    return 0;
}

@ @c
void addto_page_resources(PDF pdf, pdf_obj_type t, int k)
{
    pdf_resource_struct *re;
    pr_entry *pr, tmp;
    void **pp;
    pdf_object_list *p, *item = NULL;
    re = pdf->page_resources;
    if (re->resources_tree == NULL) {
        re->resources_tree = avl_create(comp_page_resources, NULL, &avl_xallocator);
        if (re->resources_tree == NULL)
            formatted_error("pdf backend","addto_page_resources(): avl_create() page_resource_tree failed");
    }
    tmp.obj_type = t;
    pr = (pr_entry *) avl_find(re->resources_tree, &tmp);
    if (pr == NULL) {
        pr = xtalloc(1, pr_entry);
        pr->obj_type = t;
        pr->list = NULL;
        pp = avl_probe(re->resources_tree, pr);
        if (pp == NULL)
            formatted_error("pdf backend","addto_page_resources(): avl_probe() out of memory in insertion");
    }
    if (pr->list == NULL) {
        item = xtalloc(1, pdf_object_list);
        item->link = NULL;
        item->info = k;
        pr->list = item;
        if (obj_type(pdf, k) == (int)t)
            set_obj_scheduled(pdf, k);  /* k is an object number */
    } else {
        for (p = pr->list; p->info != k && p->link != NULL; p = p->link);
        if (p->info != k) {
            item = xtalloc(1, pdf_object_list);
            item->link = NULL;
            item->info = k;
            p->link = item;
            if (obj_type(pdf, k) == (int)t)
                set_obj_scheduled(pdf, k);
        }
    }
}

@ @c
pdf_object_list *get_page_resources_list(PDF pdf, pdf_obj_type t)
{
    pdf_resource_struct *re = pdf->page_resources;
    pr_entry *pr, tmp;
    if (re == NULL || re->resources_tree == NULL)
        return NULL;
    tmp.obj_type = t;
    pr = (pr_entry *) avl_find(re->resources_tree, &tmp);
    if (pr == NULL)
        return NULL;
    return pr->list;
}

@ @c
static void reset_page_resources(PDF pdf)
{
    pdf_resource_struct *re = pdf->page_resources;
    pr_entry *p;
    struct avl_traverser t;
    pdf_object_list *l1, *l2;
    if (re == NULL || re->resources_tree == NULL)
        return;
    avl_t_init(&t, re->resources_tree);
    for (p = avl_t_first(&t, re->resources_tree); p != NULL; p = avl_t_next(&t)) {
        if (p->list != NULL) {
            for (l1 = p->list; l1 != NULL; l1 = l2) {
                l2 = l1->link;
                free(l1);
            }
            p->list = NULL;     /* but the AVL tree remains */
        }
    }
}

@ @c
static void destroy_pg_res_tree(void *pa, void *param)
{
    (void) param;
    xfree(pa);
}

@ @c
static void destroy_page_resources_tree(PDF pdf)
{
    pdf_resource_struct *re = pdf->page_resources;
    reset_page_resources(pdf);
    if (re->resources_tree != NULL)
        avl_destroy(re->resources_tree, destroy_pg_res_tree);
    re->resources_tree = NULL;
}

@* Subroutines to print out various PDF objects.

@ print out an integer |n| with fixed width |w|; used for outputting cross-reference table. The
specification says that an offset must take 10 bytes.
@c
static void pdf_print_fw_int(PDF pdf, longinteger n)
{
    unsigned char digits[11];

    int k = 10;
    do {
        k--;
        digits[k] = (unsigned char) ('0' + (n % 10));
        n /= 10;
    } while (k != 0);
    if (n!=0)
      /* the absolute  value of $n$ is greater than 9999999999 */
      normal_error("pdf backend", "offset exceeds 10 bytes, try enabling object compression.");
    digits[10]='\0';
    pdf_puts(pdf, (const char *) digits);
}

@ print out an integer |n| as a fixed number |w| of bytes; used for outputting \.{/XRef} cross-reference stream
@c
static void pdf_out_bytes(PDF pdf, longinteger n, size_t w)
{
    int k;
    unsigned char bytes[8];     /* digits in a number being output */
    k = (int) w;
    do {
        k--;
        bytes[k] = (unsigned char) (n % 256);
        n /= 256;
    } while (k != 0);
    pdf_out_block(pdf, (const char *) bytes, w);
}

@ print out |s| as string in PDF output

@c
void pdf_print_str_ln(PDF pdf, const char *s)
{
    pdf_print_str(pdf, s);
    pdf_out(pdf, '\n');
}

@ @c
void pdf_print_toks(PDF pdf, halfword p)
{
    int len = 0;
    char *s = tokenlist_to_cstring(p, true, &len);
    if (len > 0) {
        if (pdf->cave > 0)
            pdf_out(pdf, ' ');
        pdf_puts(pdf, s);
        pdf->cave = 1;
    }
    xfree(s);
}

@ prints a rect spec

@c
void pdf_add_rect_spec(PDF pdf, halfword r)
{
    pdf_add_bp(pdf, pdf_ann_left(r));
    pdf_add_bp(pdf, pdf_ann_bottom(r));
    pdf_add_bp(pdf, pdf_ann_right(r));
    pdf_add_bp(pdf, pdf_ann_top(r));
}

@ output a rectangle specification to PDF file

@c
void pdf_rectangle(PDF pdf, halfword r)
{
    pdf_add_name(pdf, "Rect");
    pdf_begin_array(pdf);
    pdf_add_rect_spec(pdf, r);
    pdf_end_array(pdf);
}

@ @c
static void init_pdf_outputparameters(PDF pdf)
{
    int pk_mode;
    pdf->draftmode = fix_int(draft_mode_par, 0, 1);
    pdf->compress_level = fix_int(pdf_compress_level, 0, 9);
    pdf->decimal_digits = fix_int(pdf_decimal_digits, 3, 5);
    pdf->gamma = fix_int(pdf_gamma, 0, 1000000);
    pdf->image_gamma = fix_int(pdf_image_gamma, 0, 1000000);
    pdf->image_hicolor = fix_int(pdf_image_hicolor, 0, 1);
    pdf->image_apply_gamma = fix_int(pdf_image_apply_gamma, 0, 1);
    pdf->objcompresslevel = fix_int(pdf_obj_compress_level, 0, MAX_OBJ_COMPRESS_LEVEL);
    pdf->inclusion_copy_font = fix_int(pdf_inclusion_copy_font, 0, 1);
    pdf->pk_resolution = fix_int(pdf_pk_resolution, 72, 8000);
    pdf->pk_fixed_dpi = fix_int(pdf_pk_fixed_dpi, 0, 1);
    if ((pdf->minor_version >= 5) && (pdf->objcompresslevel > 0)) {
        pdf->os_enable = true;
    } else {
        if (pdf->objcompresslevel > 0) {
            normal_warning("pdf backend","objcompresslevel > 0 requires minorversion > 4");
            pdf->objcompresslevel = 0;
        }
        pdf->os_enable = false;
    }
    if (pdf->pk_resolution == 0)        /* if not set from format file or by user */
        pdf->pk_resolution = pk_dpi;    /* take it from \.{texmf.cnf} */
    pdf->pk_scale_factor = divide_scaled(72, pdf->pk_resolution, pk_decimal_digits(pdf,5));
    if (!callback_defined(read_pk_file_callback)) {
        pk_mode = pdf_pk_mode; /* lookup once */
        if (pk_mode != null) {
            char *s = tokenlist_to_cstring(pk_mode, true, NULL);
            /* This will become LUATEX in 1.0. */
            kpse_init_prog("PDFTEX", (unsigned) pdf->pk_resolution, s, nil);
            xfree(s);
        } else {
            /* This will become LUATEX in 1.0. */
            kpse_init_prog("PDFTEX", (unsigned) pdf->pk_resolution, nil, nil);
        }
        if (!kpse_var_value("MKTEXPK"))
            kpse_set_program_enabled(kpse_pk_format, 1, kpse_src_cmdline);
    }
    set_job_id(pdf, year_par, month_par, day_par, time_par);
    if ((pdf_unique_resname > 0) && (pdf->resname_prefix == NULL))
        pdf->resname_prefix = get_resname_prefix(pdf);
}

@ Checks that we have a name for the generated PDF file and that it's open.

@c
static void ensure_output_file_open(PDF pdf, const char *ext)
{
    char *fn;
    if (pdf->file_name != NULL)
        return;
    if (job_name == 0)
        open_log_file();
    fn = pack_job_name(ext);
    if (pdf->draftmode == 0 || output_mode_used == OMODE_DVI) {
        while (!lua_b_open_out(&pdf->file, fn))
            fn = prompt_file_name("file name for output", ext);
    }
    pdf->file_name = fn;
}

@ @c
static void ensure_pdf_header_written(PDF pdf)
{
    /* Initialize variables for \.{PDF} output */
    fix_pdf_minorversion(pdf);
    init_pdf_outputparameters(pdf);
    fix_pdf_draftmode(pdf);
    /* Write \.{PDF} header */
    pdf_printf(pdf, "%%PDF-1.%d\n", pdf->minor_version);
    /* The next blob will be removed 1.0. */
    pdf_out(pdf, '%');
    pdf_out(pdf, 'P' + 128);
    pdf_out(pdf, 'T' + 128);
    pdf_out(pdf, 'E' + 128);
    pdf_out(pdf, 'X' + 128);
    pdf_out(pdf, '\n');
}

@ @c
void ensure_output_state(PDF pdf, output_state s)
{
    if (pdf->o_state < s) {
        if (s > ST_INITIAL)
            ensure_output_state(pdf, s - 1);
        switch (s - 1) {
            case ST_INITIAL:
                fix_o_mode();
                break;
            case ST_OMODE_FIX:
                switch (output_mode_used) {
                    case OMODE_DVI:
                        ensure_output_file_open(pdf, ".dvi");
                        break;
                    case OMODE_PDF:
                        ensure_output_file_open(pdf, ".pdf");
                        break;
                    default:
                        normal_error("pdf backend","weird output state");
                }
                break;
            case ST_FILE_OPEN:
                switch (output_mode_used) {
                    case OMODE_DVI:
                        ensure_dvi_header_written(pdf);
                        break;
                    case OMODE_PDF:
                        ensure_pdf_header_written(pdf);
                        break;
                    default:
                        normal_error("pdf backend","weird output state");
                }
                break;
            case ST_HEADER_WRITTEN:
                break;
            case ST_FILE_CLOSED:
                break;
            default:
                normal_error("pdf backend","weird output state");
        }
        pdf->o_state++;
    }
}

@ Write out an accumulated object stream.

First the object number and byte offset pairs are generated and appended to the
ready buffered object stream. By this the value of \.{/First} can be calculated.
Then a new \.{/ObjStm} object is generated, and everything is copied to the PDF
output buffer, where also compression is done. When calling this procedure,
|pdf_os_mode| must be |true|.

@c
static void pdf_os_write_objstream(PDF pdf)
{
    os_struct *os = pdf->os;
    unsigned int i, j, n1, n2;  /* n1, n2: ObjStm buffer may be reallocated! */
    strbuf_s *obuf = os->buf[OBJSTM_BUF];
    if (os->cur_objstm == 0)    /* no object stream started */
        return;
    n1 = (unsigned int) strbuf_offset(obuf);    /* remember end of collected object stream contents */
    /* this is needed here to calculate /First for the ObjStm dict */
    for (i = 0, j = 0; i < os->idx; i++) {      /* add object-number/byte-offset list to buffer */
        pdf_print_int(pdf, (int) os->obj[i].num);
        pdf_out(pdf, ' ');
        pdf_print_int(pdf, (int) os->obj[i].off);
        if (j == 9 || i == os->idx - 1) {       /* print out in groups of ten for better readability */
            pdf_out(pdf, '\n');
            j = 0;
        } else {
            pdf_out(pdf, ' ');
            j++;
        }
    }
    n2 = (unsigned int) strbuf_offset(obuf);    /* remember current buffer end */
    pdf_begin_obj(pdf, (int) os->cur_objstm, OBJSTM_NEVER);     /* switch to PDF stream writing */
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "ObjStm");
    pdf_dict_add_int(pdf, "N", (int) os->idx);  /* number of objects in ObjStm */
    pdf_dict_add_int(pdf, "First", (int) (n2 - n1));
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    /* write object-number/byte-offset list */
    pdf_out_block(pdf, (const char *) (obuf->data + n1), (size_t) (n2 - n1));
    /* write collected object stream contents */
    pdf_out_block(pdf, (const char *) obuf->data, (size_t) n1);
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    os->cur_objstm = 0;         /* to force object stream generation next time */
}

@ begin a PDF dictionary

@c
void pdf_begin_dict(PDF pdf)
{
    pdf_puts(pdf, "<<");
    pdf->cave = 0;
}

@ end a PDF dictionary

@c
void pdf_end_dict(PDF pdf)
{
    pdf_puts(pdf, ">>");
    pdf->cave = 0;
}

@ add integer object to dict

@c
void pdf_dict_add_bool(PDF pdf, const char *key, int i)
{
    pdf_add_name(pdf, key);
    pdf_add_bool(pdf, i);
}

@ add integer object to dict

@c
void pdf_dict_add_int(PDF pdf, const char *key, int i)
{
    pdf_add_name(pdf, key);
    pdf_add_int(pdf, i);
}

@ add name object to dict

@c
void pdf_dict_add_name(PDF pdf, const char *key, const char *val)
{
    pdf_add_name(pdf, key);
    pdf_add_name(pdf, val);
}

@ add string object to dict

@c
void pdf_dict_add_string(PDF pdf, const char *key, const char *val)
{
    if (val == NULL)
        return;
    pdf_add_name(pdf, key);
    if (pdf->cave > 0)
        pdf_out(pdf, ' ');
    pdf->cave = 0;
    pdf_print_str(pdf, val);
}

@ add name reference to dict

@c
void pdf_dict_add_ref(PDF pdf, const char *key, int num)
{
    pdf_add_name(pdf, key);
    pdf_add_ref(pdf, num);
}

@ add objects of different types

@c
void pdf_add_null(PDF pdf)
{
    if (pdf->cave > 0)
        pdf_out(pdf, ' ');
    pdf_puts(pdf, "null");
    pdf->cave = 1;
}

void pdf_add_bool(PDF pdf, int i)
{
    if (pdf->cave > 0)
        pdf_out(pdf, ' ');
    if (i == 0)
        pdf_puts(pdf, "false");
    else
        pdf_puts(pdf, "true");
    pdf->cave = 1;
}

void pdf_add_int(PDF pdf, int i)
{
    if (pdf->cave > 0)
        pdf_out(pdf, ' ');
    pdf_print_int(pdf, i);
    pdf->cave = 1;
}

void pdf_add_longint(PDF pdf, longinteger n)
{
    if (pdf->cave > 0)
        pdf_out(pdf, ' ');
    pdf_print_int(pdf, n);
    pdf->cave = 1;
}

void pdf_add_string(PDF pdf, const char *s)
{
    if (pdf->cave > 0)
        pdf_out(pdf, ' ');
    pdf_print_str(pdf, s);
    pdf->cave = 1;
}

void pdf_add_name(PDF pdf, const char *name)
{
    pdf_out(pdf, '/');
    pdf_puts(pdf, name);
    pdf->cave = 1;
}

void pdf_add_ref(PDF pdf, int num)
{
    if (pdf->cave > 0)
        pdf_out(pdf, ' ');
    pdf_print_int(pdf, num);
    pdf_puts(pdf, " 0 R");
    pdf->cave = 1;
}

@ add stream length and filter entries to a stream dictionary,
remember file position for seek

@c
void pdf_dict_add_streaminfo(PDF pdf)
{
    pdf_add_name(pdf, "Length");
    pdf->stream_length_offset = pdf_offset(pdf) + 1;
    pdf->seek_write_length = true; /* fill in length at |pdf_end_stream| call */
    pdf_puts(pdf, " x         ");  /* space for 10 decimal digits */
    pdf->cave = 1;
    if (pdf->compress_level > 0) {
        pdf_dict_add_name(pdf, "Filter", "FlateDecode");
        pdf->stream_deflate = true;
    }
}

@ begin a PDF array

@c
void pdf_begin_array(PDF pdf)
{
    pdf_out(pdf, '[');
    pdf->cave = 0;
}

@ end a PDF array

@c
void pdf_end_array(PDF pdf)
{
    pdf_out(pdf, ']');
    pdf->cave = 0;
}

@ begin a PDF object

@c
void pdf_begin_obj(PDF pdf, int i, int pdf_os_threshold)
{
    os_struct *os = pdf->os;
    ensure_output_state(pdf, ST_HEADER_WRITTEN);
    pdf_prepare_obj(pdf, i, pdf_os_threshold);
    switch (os->curbuf) {
        case PDFOUT_BUF:
            pdf_printf(pdf, "%d 0 obj\n", (int) i);
            break;
        case OBJSTM_BUF:
            if (pdf->compress_level == 0)
                pdf_printf(pdf, "%% %d 0 obj\n", (int) i);  /* debugging help */
            break;
        default:
            normal_error("pdf backend","weird begin object");
    }
    pdf->cave = 0;
}

@ end a PDF object

@c
void pdf_end_obj(PDF pdf)
{
    os_struct *os = pdf->os;
    switch (os->curbuf) {
        case PDFOUT_BUF:
            pdf_puts(pdf, "\nendobj\n"); /* end a PDF object */
            break;
        case OBJSTM_BUF:
            os->idx++;   /* = number of objects collected so far in ObjStm */
            os->o_ctr++; /* only for statistics */
            if (os->idx == PDF_OS_MAX_OBJS)
                pdf_os_write_objstream(pdf);
            else
                pdf_out(pdf, '\n'); /* Adobe Reader seems to need this */
            break;
        default:
            normal_error("pdf backend","weird end object");
    }
}

@ Converts any string given in in in an allowed PDF string which can be
 handled by printf et.al.: \.{\\} is escaped to \.{\\\\}, parenthesis are escaped and
 control characters are octal encoded.
 This assumes that the string does not contain any already escaped
 characters!

@c
char *convertStringToPDFString(const char *in, int len)
{
    static char pstrbuf[MAX_PSTRING_LEN];
    char *out = pstrbuf;
    int i, j, k;
    char buf[5];
    j = 0;
    for (i = 0; i < len; i++) {
        check_buf((unsigned) j + sizeof(buf), MAX_PSTRING_LEN);
        if (((unsigned char) in[i] < '!') || ((unsigned char) in[i] > '~')) {
            /* convert control characters into oct */
            k = snprintf(buf, sizeof(buf), "\\%03o", (unsigned int) (unsigned char) in[i]);
            check_nprintf(k, sizeof(buf));
            out[j++] = buf[0];
            out[j++] = buf[1];
            out[j++] = buf[2];
            out[j++] = buf[3];
        } else if ((in[i] == '(') || (in[i] == ')')) {
            /* escape paranthesis */
            out[j++] = '\\';
            out[j++] = in[i];
        } else if (in[i] == '\\') {
            /* escape backslash */
            out[j++] = '\\';
            out[j++] = '\\';
        } else {
            /* copy char :-) */
            out[j++] = in[i];
        }
    }
    out[j] = '\0';
    return pstrbuf;
}

@ Converts any string given in in in an allowed PDF string which is hexadecimal
encoded; |sizeof(out)| should be at least $|lin|*2+1$.

@c
static void convertStringToHexString(const char *in, char *out, int lin)
{
    int i, k;
    char buf[3];
    int j = 0;
    for (i = 0; i < lin; i++) {
        k = snprintf(buf, sizeof(buf), "%02X", (unsigned int) (unsigned char) in[i]);
        check_nprintf(k, sizeof(buf));
        out[j++] = buf[0];
        out[j++] = buf[1];
    }
    out[j] = '\0';
}

@ Compute the ID string as per PDF1.4 9.3:

File identifers are defined by the optional ID entry in a PDF file's trailer
dictionary (see Section 3.4.4, "File Trailer"; see also implementation note 105
in Appendix H). The value of this entry is an array of two strings. The first
string is a permanent identifier based on the contents of the file at the time it
was originally created, and does not change when the file is incrementally
updated. The second string is a changing identifier based on the file's contents
at the time it was last updated. When a file is first written, both identifiers
are set to the same value. If both identifiers match when a file reference is
resolved, it is very likely that the correct file has been found; if only the
first identifier matches, then a different version of the correct file has been
found. To help ensure the uniqueness of file identifiers, it is recommend that
they be computed using a message digest algorithm such as MD5 (described in
Internet RFC 1321, The MD5 Message-Digest Algorithm; see the Bibliography), using
the following information (see implementation note 106 in Appendix H): - The
current time

- A string representation of the file's location, usually a pathname
- The size of the file in bytes
- The values of all entries in the file's document information
  dictionary (see Section 9.2.1,  Document Information Dictionary )

This stipulates only that the two IDs must be identical when the file is created
and that they should be reasonably unique. Since it's difficult to get the file
size at this point in the execution of pdfTeX and scanning the info dict is also
difficult, we start with a simpler implementation using just the first two items.


@c

/*
    A user supplied trailerid had better be an array! So maybe we need to check
    for [] and error otherwise.
*/

static void print_ID(PDF pdf)
{
    if ((pdf_suppress_optional_info & 512) == 0) {
        const char *p = NULL;
        pdf_add_name(pdf, "ID");
        p = get_pdf_table_string("trailerid");
        if (p && strlen(p) > 0) {
            pdf_puts(pdf,p);
        } else if (pdf_trailer_id != 0) {
            /* user provided one */
            pdf_print_toks(pdf, pdf_trailer_id);
        } else {
            /* system provided one */
            time_t t;
            size_t size;
            char time_str[32];
            md5_state_t state;
            md5_byte_t digest[16];
            char id[64];
            char pwd[4096];
            /* start md5 */
            md5_init(&state);
            /* get the time */
            t = pdf->start_time;
            size = strftime(time_str, sizeof(time_str), "%Y%m%dT%H%M%SZ", gmtime(&t));
            md5_append(&state, (const md5_byte_t *) time_str, (int) size);
            /* get the file name */
            if (getcwd(pwd, sizeof(pwd)) == NULL) {
                formatted_error("pdf backend","getcwd() failed (%s), (path too long?)", strerror(errno));
            }
#ifdef WIN32
            {
                char *p;
                for (p = pwd; *p; p++) {
                    if (*p == '\\')
                        *p = '/';
#if !defined(MIKTEX)
                    else if (IS_KANJI(p))
                        p++;
#endif
                }
            }
#endif
            md5_append(&state, (const md5_byte_t *) pwd, (int) strlen(pwd));
            md5_append(&state, (const md5_byte_t *) "/", 1);
            md5_append(&state, (const md5_byte_t *) pdf->file_name, (int) strlen(pdf->file_name));
            /* finish md5 */
            md5_finish(&state, digest);
            /* write the IDs */
            convertStringToHexString((char *) digest, id, 16);
            pdf_begin_array(pdf);
            pdf_printf(pdf, "<%s> <%s>", id, id);
            pdf_end_array(pdf);
        }
    }
}

@ Print the /CreationDate entry.

PDF Reference, third edition says about the expected date format:

3.8.2 Dates

PDF defines a standard date format, which closely follows that of the
international standard ASN.1 (Abstract Syntax Notation One), defined in ISO/IEC
8824 (see the Bibliography). A date is a string of the form

(D:YYYYMMDDHHmmSSOHH'mm')

where

YYYY  is the year
MM    is the month
DD    is the day (01-31)
HH    is the hour (00-23)
mm    is the minute (00-59)
SS    is the second (00-59)
O     is the relationship of local time to Universal Time (UT), denoted by one
      of the characters +, -, or Z (see below)
HH    followed by ' is the absolute value of the offset from UT in hours (00-23)
mm    followed by ' is the absolute value of the offset from UT in minutes (00-59)

The apostrophe character (') after HH and mm is part of the syntax. All fields
after the year are optional. (The prefix D:, although also optional, is strongly
recommended.) The default values for MM and DD are both 01; all other numerical
fields default to zero values. A plus sign (+) as the value of the O field
signifies that local time is later than UT, a minus sign (-) that local time is
earlier than UT, and the letter Z that local time is equal to UT. If no UT
information is specified, the relationship of the specified time to UT is
considered to be unknown. Whether or not the time zone is known, the rest of the
date should be specified in local time.

For example, December 23, 1998, at 7:52 PM, U.S. Pacific Standard Time, is
represented by the string

D:199812231952-08'00'

The main difficulty is get the time zone offset. |strftime()| does this in ISO
C99 (e.g. newer glibc) with \%z, but we have to work with other systems (e.g.
Solaris 2.5).

@c
#define TIME_STR_SIZE 30 /* minimum size for |time_str| is 24: |"D:YYYYmmddHHMMSS+HH'MM'"| */

static void makepdftime(PDF pdf)
{
    struct tm lt, gmt;
    size_t size;
    int i, off, off_hours, off_mins;
    time_t t = pdf->start_time;
    char *time_str = pdf->start_time_str;
    /* get the time */
    if (utc_option) {
        lt = *gmtime(&t);
    } else {
        lt = *localtime(&t);
    }
    size = strftime(time_str, TIME_STR_SIZE, "D:%Y%m%d%H%M%S", &lt);
    /* expected format: "YYYYmmddHHMMSS" */
    if (size == 0) {
        /* unexpected, contents of |time_str| is undefined */
        time_str[0] = '\0';
        return;
    }
    /*
       correction for seconds: \%S can be in range 00..61, the PDF reference
       expects 00..59, therefore we map "60" and "61" to "59"
    */
    if (time_str[14] == '6') {
        time_str[14] = '5';
        time_str[15] = '9';
        /* for safety */
        time_str[16] = '\0';
    }
    /* get the time zone offset */
    gmt = *gmtime(&t);
    /* this calculation method was found in exim's tod.c */
    off = 60 * (lt.tm_hour - gmt.tm_hour) + lt.tm_min - gmt.tm_min;
    if (lt.tm_year != gmt.tm_year) {
        off += (lt.tm_year > gmt.tm_year) ? 1440 : -1440;
    } else if (lt.tm_yday != gmt.tm_yday) {
        off += (lt.tm_yday > gmt.tm_yday) ? 1440 : -1440;
    }
    if (off == 0) {
        time_str[size++] = 'Z';
        time_str[size] = 0;
    } else {
        off_hours = off / 60;
        off_mins = abs(off - off_hours * 60);
        i = snprintf(&time_str[size], 9, "%+03d'%02d'", off_hours, off_mins);
        check_nprintf(i, 9);
    }
    pdf->start_time = t;
}

@ @c
void initialize_start_time(PDF pdf)
{
    if (pdf->start_time == 0) {
        pdf->start_time = get_start_time();
        pdf->start_time_str = xtalloc(TIME_STR_SIZE, char);
        makepdftime(pdf);
    }
}

@ @c
char *getcreationdate(PDF pdf)
{
    initialize_start_time(pdf);
    return pdf->start_time_str;
}

@ @c
void remove_pdffile(PDF pdf)
{
    if (pdf != NULL) {
        if (!kpathsea_debug && pdf->file_name && (pdf->draftmode == 0)) {
            xfclose(pdf->file, pdf->file_name);
            remove(pdf->file_name);
        }
    }
}

@ Use |check_o_mode()| in the backend-specific "Implement..." chunks

@c
void check_o_mode(PDF pdf, const char *s, int o_mode_bitpattern, boolean strict) /* s ignored now */
{

    output_mode o_mode;
    const char *m = NULL;
    if (lua_only) {
        normal_error("lua only","no backend present, needed for what you asked for");
        return ;
    }
    /*
        in warn mode (strict == false): only check, don't do |fix_o_mode()| here! |output_mode_used|
        is left in possibly wrong state until real output, ok.
    */
    if (output_mode_used == OMODE_NONE)
        o_mode = get_o_mode();
    else
        o_mode = output_mode_used;
    pdf->o_mode = output_mode_used; /* used by synctex, we need to use output_mode_used there */
    if (!((1 << o_mode) & o_mode_bitpattern)) { /* warning or error */
        switch (o_mode) {
            case OMODE_DVI:
                m = "DVI";
                break;
            case OMODE_PDF:
                m = "PDF";
                break;
            default:
               normal_error("pdf backend","weird output state");
         }
        if (strict)
            formatted_error("pdf backend", "%s not allowed in %s mode (outputmode = %d)",s, m, (int) output_mode_par);
        else
            formatted_warning("pdf backend", "%s not allowed in %s mode (outputmode = %d)",s, m, (int) output_mode_par);
    } else if (strict)
        ensure_output_state(pdf, ST_HEADER_WRITTEN);
}

@ @c
void set_job_id(PDF pdf, int year, int month, int day, int time)
{
    char *name_string, *format_string, *s;
    size_t slen;
    int i;
    if (pdf->job_id_string != NULL)
        return;
    name_string = makecstring(job_name);
    format_string = makecstring(format_ident);
    slen = SMALL_BUF_SIZE + strlen(name_string) + strlen(format_string) + strlen(luatex_banner);
    s = xtalloc(slen, char);
    /* The Web2c version string starts with a space.  */
    i = snprintf(s, slen, "%.4d/%.2d/%.2d %.2d:%.2d %s %s %s", year, month, day, time / 60, time % 60, name_string, format_string, luatex_banner);
    check_nprintf(i, slen);
    pdf->job_id_string = xstrdup(s);
    xfree(s);
    xfree(name_string);
    xfree(format_string);
}

@ @c
char *get_resname_prefix(PDF pdf)
{
    static char name_str[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static char prefix[7]; /* make a tag of 6 chars long */
    short i;
    size_t base = strlen(name_str);
    unsigned long crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (Bytef *) pdf->job_id_string, (uInt) strlen(pdf->job_id_string));
    for (i = 0; i < 6; i++) {
        prefix[i] = name_str[crc % base];
        crc /= base;
    }
    prefix[6] = '\0';
    return prefix;
}

@ @c
void pdf_begin_page(PDF pdf)
{
    int xform_attributes;
    int xform_type = 0;
    scaled form_margin = pdf_xform_margin; /* was one_bp until SVN4066 */
    ensure_output_state(pdf, ST_HEADER_WRITTEN);
    init_pdf_pagecalculations(pdf);
    if (pdf->page_resources == NULL) {
        pdf->page_resources = xtalloc(1, pdf_resource_struct);
        pdf->page_resources->resources_tree = NULL;
    }
    pdf->page_resources->last_resources = pdf_create_obj(pdf, obj_type_others, 0);
    reset_page_resources(pdf);

    if (global_shipping_mode == SHIPPING_PAGE) {
        pdf->last_page = pdf_get_obj(pdf, obj_type_page, total_pages + 1, 0);
        set_obj_aux(pdf, pdf->last_page, 1);    /* mark that this page has been created */
        pdf->last_stream = pdf_create_obj(pdf, obj_type_pagestream, 0);
        pdf_begin_obj(pdf, pdf->last_stream, OBJSTM_NEVER);
        pdf->last_thread = null;
        pdf_begin_dict(pdf);
    } else {
        xform_type = obj_xform_type(pdf, pdf_cur_form) ;
        pdf_begin_obj(pdf, pdf_cur_form, OBJSTM_NEVER);
        pdf->last_stream = pdf_cur_form;
        /* Write out Form stream header */
        pdf_begin_dict(pdf);
        if (xform_type == 0) {
            pdf_dict_add_name(pdf, "Type", "XObject");
            pdf_dict_add_name(pdf, "Subtype", "Form");
            pdf_dict_add_int(pdf, "FormType", 1);
        }
        xform_attributes = pdf_xform_attr; /* lookup once */
        if (xform_attributes != null)
            pdf_print_toks(pdf, xform_attributes);
        if (obj_xform_attr(pdf, pdf_cur_form) != null) {
            pdf_print_toks(pdf, obj_xform_attr(pdf, pdf_cur_form));
            delete_token_ref(obj_xform_attr(pdf, pdf_cur_form));
            set_obj_xform_attr(pdf, pdf_cur_form, null);
        }
        if (obj_xform_attr_str(pdf, pdf_cur_form) != null) {
            lua_pdf_literal(pdf, obj_xform_attr_str(pdf, pdf_cur_form));
            luaL_unref(Luas, LUA_REGISTRYINDEX, obj_xform_attr_str(pdf, pdf_cur_form));
            set_obj_xform_attr_str(pdf, pdf_cur_form, null);
        }
        if (xform_type == 0 || xform_type == 1 || xform_type == 3) {
            pdf_add_name(pdf, "BBox");
            pdf_begin_array(pdf);
            pdf_add_bp(pdf, -form_margin);
            pdf_add_bp(pdf, -form_margin);
            pdf_add_bp(pdf, pdf->page_size.h + form_margin);
            pdf_add_bp(pdf, pdf->page_size.v + form_margin);
            pdf_end_array(pdf);
        }
        if (xform_type == 0 || xform_type == 2 || xform_type == 3) {
            pdf_add_name(pdf, "Matrix");
            pdf_begin_array(pdf);
            pdf_add_int(pdf, 1);
            pdf_add_int(pdf, 0);
            pdf_add_int(pdf, 0);
            pdf_add_int(pdf, 1);
            pdf_add_int(pdf, 0);
            pdf_add_int(pdf, 0);
            pdf_end_array(pdf);
        }
        pdf_dict_add_ref(pdf, "Resources", pdf->page_resources->last_resources);
    }
    /* Start stream of page/form contents */
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    pos_stack_used = 0; /* start with empty stack */
    if (global_shipping_mode == SHIPPING_PAGE) {
        colorstackpagestart();
    }
    if (global_shipping_mode == SHIPPING_PAGE)
        pdf_out_colorstack_startpage(pdf);
}

@ @c
void print_pdf_table_string(PDF pdf, const char *s)
{
    size_t len;
    const char *ls;
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, lua_key_index(pdf_data));
    lua_rawget(Luas, LUA_REGISTRYINDEX);
    lua_pushstring(Luas, s);                  /* s t ... */
    lua_rawget(Luas, -2);                     /* s? t ... */
    if (lua_type(Luas, -1) == LUA_TSTRING) {  /* s t ... */
        ls = lua_tolstring(Luas, -1, &len);
        if (len > 0) {
            if (pdf->cave == 1)
                pdf_out(pdf, ' ');
            pdf_out_block(pdf, ls, len);
            pdf->cave = 1;
        }
    }
    lua_pop(Luas, 2);
}

@ @c
const char *get_pdf_table_string(const char *s)
{
    const_lstring ls;
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, lua_key_index(pdf_data));
    lua_rawget(Luas, LUA_REGISTRYINDEX);
    lua_pushstring(Luas, s);                   /* s t ... */
    lua_rawget(Luas, -2);                      /* s? t ... */
    if (lua_type(Luas, -1) == LUA_TSTRING) {   /* s t ... */
        ls.s = lua_tolstring(Luas, -1, &ls.l);
        /*
            s is supposed to be anchored (e.g in the registry)
            so it's not garbage collected
        */
        lua_pop(Luas, 2);
        return ls.s;
    }
    lua_pop(Luas, 2);
    return NULL ;
}

@ @c
void pdf_end_page(PDF pdf)
{
    char s[64], *p;
    int j, annots = 0, beads = 0, callback_id;
    pdf_resource_struct *res_p = pdf->page_resources;
    pdf_resource_struct local_page_resources;
    pdf_object_list *annot_list, *bead_list, *link_list, *ol, *ol1;
    scaledpos save_cur_page_size; /* to save |pdf->page_size| during flushing pending forms */
    shipping_mode_e save_shipping_mode;
    int xform_resources;
    int page_resources, page_attributes;
    int procset = PROCSET_PDF;
    /* Finish stream of page/form contents */
    pdf_goto_pagemode(pdf);
    if (pos_stack_used > 0) {
        formatted_error("pdf backend","%u unmatched 'save' after %s shipout", (unsigned int) pos_stack_used,
            ((global_shipping_mode == SHIPPING_PAGE) ? "page" : "form"));
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    callback_id = callback_defined(finish_pdfpage_callback);
    if (callback_id > 0)
      run_callback(callback_id, "b->",(global_shipping_mode == SHIPPING_PAGE));
    if (global_shipping_mode == SHIPPING_PAGE) {
        pdf->last_pages = pdf_do_page_divert(pdf, pdf->last_page, 0);
        /* Write out /Page object */
        pdf_begin_obj(pdf, pdf->last_page, OBJSTM_ALWAYS);
        pdf_begin_dict(pdf);
        pdf_dict_add_name(pdf, "Type", "Page");
        pdf_dict_add_ref(pdf, "Contents", pdf->last_stream);
        pdf_dict_add_ref(pdf, "Resources", res_p->last_resources);
        pdf_add_name(pdf, "MediaBox");
        pdf_begin_array(pdf);
        pdf_add_int(pdf, 0);
        pdf_add_int(pdf, 0);
        pdf_add_bp(pdf, pdf->page_size.h);
        pdf_add_bp(pdf, pdf->page_size.v);
        pdf_end_array(pdf);
        page_attributes = pdf_page_attr ; /* lookup once */
        if (page_attributes != null)
            pdf_print_toks(pdf, page_attributes);
        print_pdf_table_string(pdf, "pageattributes");
        pdf_dict_add_ref(pdf, "Parent", pdf->last_pages);
        if (pdf->img_page_group_val != 0) {
            pdf_dict_add_ref(pdf, "Group", pdf->img_page_group_val);
        }
        annot_list = get_page_resources_list(pdf, obj_type_annot);
        link_list = get_page_resources_list(pdf, obj_type_link);
        if (annot_list != NULL || link_list != NULL) {
            annots = pdf_create_obj(pdf, obj_type_annots, 0);
            pdf_dict_add_ref(pdf, "Annots", annots);
        }
        bead_list = get_page_resources_list(pdf, obj_type_bead);
        if (bead_list != NULL) {
            beads = pdf_create_obj(pdf, obj_type_beads, 0);
            pdf_dict_add_ref(pdf, "B", beads);
        }
        pdf_end_dict(pdf);
        pdf_end_obj(pdf);
        pdf->img_page_group_val = 0;
        /* Generate array of annotations or beads in page */
        if (annot_list != NULL || link_list != NULL) {
            pdf_begin_obj(pdf, annots, OBJSTM_ALWAYS);
            pdf_begin_array(pdf);
            while (annot_list != NULL) {
                pdf_add_ref(pdf, annot_list->info);
                annot_list = annot_list->link;
            }
            while (link_list != NULL) {
                pdf_add_ref(pdf, link_list->info);
                link_list = link_list->link;
            }
            pdf_end_array(pdf);
            pdf_end_obj(pdf);
        }
        if (bead_list != NULL) {
            pdf_begin_obj(pdf, beads, OBJSTM_ALWAYS);
            pdf_begin_array(pdf);
            while (bead_list != NULL) {
                pdf_add_ref(pdf, bead_list->info);
                bead_list = bead_list->link;
            }
            pdf_end_array(pdf);
            pdf_end_obj(pdf);
        }
    }
    /* Write out resource lists and pending raw objects */
    ol = get_page_resources_list(pdf, obj_type_obj);
    while (ol != NULL) {
        if (!is_obj_written(pdf, ol->info))
            pdf_write_obj(pdf, ol->info);
        ol = ol->link;
    }

    /*
        When flushing pending forms we need to save and restore resource lists
        which are also used by page shipping. Saving and restoring
        |pdf->page_size| is needed for proper writing out pending PDF marks.
    */
    ol = get_page_resources_list(pdf, obj_type_xform);
    while (ol != NULL) {
        if (!is_obj_written(pdf, ol->info)) {
            pdf_cur_form = ol->info;
            save_cur_page_size = pdf->page_size;
            save_shipping_mode = global_shipping_mode;
            pdf->page_resources = &local_page_resources;
            local_page_resources.resources_tree = NULL;
            ship_out(pdf, obj_xform_box(pdf, pdf_cur_form), SHIPPING_FORM);
            /* Restore page size and page resources */
            pdf->page_size = save_cur_page_size;
            global_shipping_mode = save_shipping_mode;
            destroy_page_resources_tree(pdf);
            pdf->page_resources = res_p;
        }
        ol = ol->link;
    }
    /* Write out pending images */
    ol = get_page_resources_list(pdf, obj_type_ximage);
    while (ol != NULL) {
        if (!is_obj_written(pdf, ol->info))
            pdf_write_image(pdf, ol->info);
        ol = ol->link;
    }
    if (global_shipping_mode == SHIPPING_PAGE) {
        /* Write out pending PDF marks and annotations */
        ol = get_page_resources_list(pdf, obj_type_annot);
        while (ol != NULL) {
            if (ol->info > 0 && obj_type(pdf, ol->info) == obj_type_annot) {
                j = obj_annot_ptr(pdf, ol->info); /* |j| points to |pdf_annot_node| */
                pdf_begin_obj(pdf, ol->info, OBJSTM_ALWAYS);
                pdf_begin_dict(pdf);
                pdf_dict_add_name(pdf, "Type", "Annot");
                pdf_print_toks(pdf, pdf_annot_data(j));
                pdf_rectangle(pdf, j);
                pdf_end_dict(pdf);
                pdf_end_obj(pdf);
            }
            ol = ol->link;
        }
        /* Write out PDF link annotations */
        if ((ol = get_page_resources_list(pdf, obj_type_link)) != NULL) {
            while (ol != NULL) {
                j = obj_annot_ptr(pdf, ol->info);
                pdf_begin_obj(pdf, ol->info, OBJSTM_ALWAYS);
                pdf_begin_dict(pdf);
                pdf_dict_add_name(pdf, "Type", "Annot");
                if (pdf_action_type(pdf_link_action(j)) != pdf_action_user)
                    pdf_dict_add_name(pdf, "Subtype", "Link");
                if (pdf_link_attr(j) != null)
                    pdf_print_toks(pdf, pdf_link_attr(j));
                pdf_rectangle(pdf, j);
                if (pdf_action_type(pdf_link_action(j)) != pdf_action_user)
                    pdf_puts(pdf, "/A ");
                write_action(pdf, pdf_link_action(j));
                pdf_end_dict(pdf);
                pdf_end_obj(pdf);
                ol = ol->link;
            }
            /* Flush |pdf_start_link_node|'s created by |append_link| */
            ol = get_page_resources_list(pdf, obj_type_link);
            while (ol != NULL) {
                j = obj_annot_ptr(pdf, ol->info);
                /*
                    nodes with |subtype = pdf_link_data_node| were created by |append_link| and
                    must be flushed here, as they are not linked in any list
                */
                if (subtype(j) == pdf_link_data_node)
                    flush_node(j);
                ol = ol->link;
            }
        }
        /* Write out PDF mark destinations */
        write_out_pdf_mark_destinations(pdf);
        /* Write out PDF bead rectangle specifications */
        print_bead_rectangles(pdf);
    }
    /* Write out resources dictionary */
    pdf_begin_obj(pdf, res_p->last_resources, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    /* Print additional resources */
    if (global_shipping_mode == SHIPPING_PAGE) {
        page_resources = pdf_page_resources; /* lookup once */
        if (page_resources != null)
            pdf_print_toks(pdf, page_resources);
        print_pdf_table_string(pdf, "pageresources");
    } else {
        xform_resources = pdf_xform_resources; /* lookup once */
        if (xform_resources != null)
            pdf_print_toks(pdf, xform_resources);
        if (obj_xform_resources(pdf, pdf_cur_form) != null) {
            pdf_print_toks(pdf, obj_xform_resources(pdf, pdf_cur_form));
            delete_token_ref(obj_xform_resources(pdf, pdf_cur_form));
            set_obj_xform_resources(pdf, pdf_cur_form, null);
        }
        if (obj_xform_resources_str(pdf, pdf_cur_form) != null) {
            lua_pdf_literal(pdf, obj_xform_resources_str(pdf, pdf_cur_form));
            luaL_unref(Luas, LUA_REGISTRYINDEX, obj_xform_resources_str(pdf, pdf_cur_form));
            set_obj_xform_resources_str(pdf, pdf_cur_form, null);
        }
    }
    /* Generate font resources */
    if ((ol = get_page_resources_list(pdf, obj_type_font)) != NULL) {
        pdf_add_name(pdf, "Font");
        pdf_begin_dict(pdf);
        while (ol != NULL) {
            p = s;
            p += snprintf(p, 20, "F%i", obj_info(pdf, ol->info));
            if (pdf->resname_prefix != NULL)
                p += snprintf(p, 20, "%s", pdf->resname_prefix);
            pdf_dict_add_ref(pdf, s, ol->info);
            ol = ol->link;
        }
        pdf_end_dict(pdf);
        procset |= PROCSET_TEXT;
    }
    /* Generate XObject resources */
    ol = get_page_resources_list(pdf, obj_type_xform);
    ol1 = get_page_resources_list(pdf, obj_type_ximage);
    if (ol != NULL || ol1 != NULL) {
        pdf_add_name(pdf, "XObject");
        pdf_begin_dict(pdf);
        while (ol != NULL) {
            p = s;
            p += snprintf(p, 20, "Fm%i", obj_info(pdf, ol->info));
            if (pdf->resname_prefix != NULL)
                p += snprintf(p, 20, "%s", pdf->resname_prefix);
            pdf_dict_add_ref(pdf, s, ol->info);
            ol = ol->link;
        }
        while (ol1 != null) {
            p = s;
            p += snprintf(p, 20, "Im%i", obj_info(pdf, ol1->info));
            if (pdf->resname_prefix != NULL)
                p += snprintf(p, 20, "%s", pdf->resname_prefix);
            pdf_dict_add_ref(pdf, s, ol1->info);
            procset |= img_procset(idict_array[obj_data_ptr(pdf, ol1->info)]);
            ol1 = ol1->link;
        }
        pdf_end_dict(pdf);
    }
    /* Generate ProcSet */
    pdf_add_name(pdf, "ProcSet");
    pdf_begin_array(pdf);
    if ((procset & PROCSET_PDF) != 0)
        pdf_add_name(pdf, "PDF");
    if ((procset & PROCSET_TEXT) != 0)
        pdf_add_name(pdf, "Text");
    if ((procset & PROCSET_IMAGE_B) != 0)
        pdf_add_name(pdf, "ImageB");
    if ((procset & PROCSET_IMAGE_C) != 0)
        pdf_add_name(pdf, "ImageC");
    if ((procset & PROCSET_IMAGE_I) != 0)
        pdf_add_name(pdf, "ImageI");
    pdf_end_array(pdf);
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
}

@* Finishing the PDF output file.

@ Destinations that have been referenced but don't exists have
|obj_dest_ptr=null|. Leaving them undefined might cause troubles for PDF
browsers, so we need to fix them; they point to the last page.

@c
static void check_nonexisting_destinations(PDF pdf)
{
    int k;
    for (k = pdf->head_tab[obj_type_dest]; k != 0; k = obj_link(pdf, k)) {
        if (obj_dest_ptr(pdf, k) == null) {
            if (obj_info(pdf, k) < 0) {
                char *ss = makecstring(-obj_info(pdf, k));
                formatted_warning("pdf backend", "unreferenced destination with name '%s'",ss);
            } else {
                formatted_warning("pdf backend", "unreferenced destination with num '%d'",obj_info(pdf,k));
            }

            pdf_begin_obj(pdf, k, OBJSTM_ALWAYS);
            pdf_begin_array(pdf);
            pdf_add_ref(pdf, pdf->last_page);
            pdf_add_name(pdf, "Fit");
            pdf_end_array(pdf);
            pdf_end_obj(pdf);
        }
    }
}

@ @c
static void check_nonexisting_pages(PDF pdf)
{
    struct avl_traverser t;
    oentry *p;
    struct avl_table *page_tree = pdf->obj_tree[obj_type_page];
    avl_t_init(&t, page_tree);
    /* search from the end backward until the last real page is found */
    for (p = avl_t_last(&t, page_tree); p != NULL && obj_aux(pdf, p->objptr) == 0; p = avl_t_prev(&t)) {
        formatted_warning("pdf backend", "page %d has been referenced but does not exist",obj_info(pdf, p->objptr));
    }
}

@ If the same keys in a dictionary are given several times, then it is not
defined which value is choosen by an application. Therefore the keys |/Producer|
and |/Creator| are only set if the token list |pdf_info_toks| converted to a
string does not contain these key strings.

@c
static boolean substr_of_str(const char *s, const char *t)
{
    if (strstr(t, s) == NULL)
        return false;
    return true;
}

static int pdf_print_info(PDF pdf, int luatexversion,
                          str_number luatexrevision)
{                               /* print info object */
    boolean creator_given = false;
    boolean producer_given = false;
    boolean creationdate_given = false;
    boolean moddate_given = false;
    boolean trapped_given = false;
    char *s = NULL;
    const char *p = NULL;
    int k, len = 0;
    k = pdf_create_obj(pdf, obj_type_info, 0);
    pdf_begin_obj(pdf, k, 3);   /* keep Info readable unless explicitely forced */
    pdf_begin_dict(pdf);
    if (pdf_info_toks != 0) {
        s = tokenlist_to_cstring(pdf_info_toks, true, &len);
        creator_given = substr_of_str("/Creator", s);
        producer_given = substr_of_str("/Producer", s);
        creationdate_given = substr_of_str("/CreationDate", s);
        moddate_given = substr_of_str("/ModDate", s);
        trapped_given = substr_of_str("/Trapped", s);
    }
    p = get_pdf_table_string("info");
    if (p && strlen(p) > 0) {
        creator_given = creator_given || substr_of_str("/Creator", p);
        producer_given = producer_given || substr_of_str("/Producer", p);
        creationdate_given = creationdate_given || substr_of_str("/CreationDate", p);
        moddate_given = moddate_given || substr_of_str("/ModDate", p);
        trapped_given = trapped_given || substr_of_str("/Trapped", p);
    }
    if (pdf_info_toks != null) {
        if (len > 0) {
            pdf_out(pdf, '\n');
            pdf_puts(pdf, s);
            pdf_out(pdf, '\n');
            xfree(s);
        }
        delete_token_ref(pdf_info_toks);
        pdf_info_toks = null;
    }
    if (p && strlen(p) > 0) {
        pdf_out(pdf, '\n');
        pdf_puts(pdf, p); /* no free, pointer */
        pdf_out(pdf, '\n');
    }
    if ((pdf_suppress_optional_info & 128) == 0 && !producer_given) {
        pdf_add_name(pdf, "Producer");
        pdf_puts(pdf, " (LuaTeX-");
        pdf_print_int(pdf, luatexversion / 100);
        pdf_out(pdf, '.');
        pdf_print_int(pdf, luatexversion % 100);
        pdf_out(pdf, '.');
        pdf_print(pdf, luatexrevision);
        pdf_out(pdf, ')');
    }
    if ((pdf_suppress_optional_info & 16) == 0 && !creator_given) {
        pdf_dict_add_string(pdf, "Creator", "TeX");
    }
    if ((pdf_suppress_optional_info & 32) == 0 && !creationdate_given) {
        initialize_start_time(pdf);
        pdf_dict_add_string(pdf, "CreationDate", pdf->start_time_str);
    }
    if ((pdf_suppress_optional_info & 64) == 0 && !moddate_given) {
        initialize_start_time(pdf);
        pdf_dict_add_string(pdf, "ModDate", pdf->start_time_str);
    }
    if ((pdf_suppress_optional_info & 256) == 0 && !trapped_given) {
        pdf_dict_add_name(pdf, "Trapped", "False");
    }
    if ((pdf_suppress_optional_info & 1) == 0) {
        pdf_dict_add_string(pdf, "PTEX.FullBanner", luatex_banner);
    }
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
    return k;
}

static void build_free_object_list(PDF pdf)
{
    int k;
    int l = 0;
    set_obj_fresh(pdf, l);      /* null object at begin of list of free objects */
    for (k = 1; k <= pdf->obj_ptr; k++) {
        if (!is_obj_written(pdf, k)) {
            set_obj_link(pdf, l, k);
            l = k;
        }
    }
    set_obj_link(pdf, l, 0);
}

@ Now the finish of PDF output file. At this moment all Page objects
are already written completely to PDF output file.

@c
void finish_pdf_file(PDF pdf, int luatexversion, str_number luatexrevision)
{
    int i, j, k;
    int root, info, xref_stm = 0, outlines, threads, names_tree;
    size_t xref_offset_width;
    int callback_id = callback_defined(stop_run_callback);
    int callback_id1 = callback_defined(finish_pdffile_callback);
    if (total_pages == 0) {
        if (callback_id == 0) {
            normal_warning("pdf backend","no pages of output.");
        } else if (callback_id > 0) {
            run_callback(callback_id, "->");
        }
        if (pdf->gone > 0)
            normal_error("pdf backend","dangling objects discarded, no output file produced.");
    } else {
        if (pdf->draftmode == 0) {
            pdf_flush(pdf); /* to make sure that the output file name has been already created */
            flush_jbig2_page0_objects(pdf); /* flush page 0 objects from JBIG2 images, if any */
            if (callback_id1 > 0)
                run_callback(callback_id1, "->");
            check_nonexisting_pages(pdf);
            check_nonexisting_destinations(pdf);
            /* Output fonts definition */
            for (k = 1; k <= max_font_id(); k++) {
                if (font_used(k) && (pdf_font_num(k) < 0)) {
                    i = -pdf_font_num(k);
                    for (j = font_bc(k); j <= font_ec(k); j++)
                        if (quick_char_exists(k, j) && pdf_char_marked(k, j))
                            pdf_mark_char(i, j);
                    if ((pdf_font_attr(i) == 0) && (pdf_font_attr(k) != 0)) {
                        set_pdf_font_attr(i, pdf_font_attr(k));
                    } else if ((pdf_font_attr(k) == 0) && (pdf_font_attr(i) != 0)) {
                        set_pdf_font_attr(k, pdf_font_attr(i));
                    } else if ((pdf_font_attr(i) != 0) && (pdf_font_attr(k) != 0) && (!str_eq_str(pdf_font_attr(i), pdf_font_attr(k)))) {
                        formatted_warning("pdf backend","fontattr of font %d and %d are conflicting, %k is used",i,k,i);
                    }
                }
            }
            pdf->gen_tounicode = pdf_gen_tounicode;
            pdf->omit_cidset = pdf_omit_cidset;
            k = pdf->head_tab[obj_type_font];
            while (k != 0) {
                int f = obj_info(pdf, k);
                do_pdf_font(pdf, f);
                k = obj_link(pdf, k);
            }
            write_fontstuff(pdf);
            pdf->last_pages = output_pages_tree(pdf);
            /* Output outlines */
            outlines = print_outlines(pdf);
            /*
                Output name tree. The name tree is very similiar to Pages tree so
                its construction should be certain from Pages tree construction.
                For intermediate node |obj_info| will be the first name and
                |obj_link| will be the last name in \.{\\Limits} array. Note that
                |pdf_dest_names_ptr| will be less than |obj_ptr|, so we test if
                |k < pdf_dest_names_ptr| then |k| is index of leaf in
                |dest_names|; else |k| will be index in |obj_tab| of some
                intermediate node.
             */
            names_tree = output_name_tree(pdf);

            /* Output article threads */
            if (pdf->head_tab[obj_type_thread] != 0) {
                threads = pdf_create_obj(pdf, obj_type_others, 0);
                pdf_begin_obj(pdf, threads, OBJSTM_ALWAYS);
                pdf_begin_array(pdf);
                k = pdf->head_tab[obj_type_thread];
                while (k != 0) {
                    pdf_add_ref(pdf, k);
                    k = obj_link(pdf, k);
                }
                pdf_end_array(pdf);
                pdf_end_obj(pdf);
                k = pdf->head_tab[obj_type_thread];
                while (k != 0) {
                    out_thread(pdf, k);
                    k = obj_link(pdf, k);
                }
            } else {
                threads = 0;
            }
            /* Output the /Catalog object */
            root = pdf_create_obj(pdf, obj_type_catalog, 0);
            pdf_begin_obj(pdf, root, OBJSTM_ALWAYS);
            pdf_begin_dict(pdf);
            pdf_dict_add_name(pdf, "Type", "Catalog");
            pdf_dict_add_ref(pdf, "Pages", pdf->last_pages);
            if (threads != 0)
                pdf_dict_add_ref(pdf, "Threads", threads);
            if (outlines != 0)
                pdf_dict_add_ref(pdf, "Outlines", outlines);
            if (names_tree != 0)
                pdf_dict_add_ref(pdf, "Names", names_tree);
            if (pdf_catalog_toks != null) {
                pdf_print_toks(pdf, pdf_catalog_toks);
                delete_token_ref(pdf_catalog_toks);
                pdf_catalog_toks = null;
            }
            print_pdf_table_string(pdf, "catalog");
            if (pdf_catalog_openaction != 0)
                pdf_dict_add_ref(pdf, "OpenAction", pdf_catalog_openaction);
            pdf_end_dict(pdf);
            pdf_end_obj(pdf);
            /* last candidate for object stream */
            info = pdf_print_info(pdf, luatexversion, luatexrevision);
            /* final object for pdf->os_enable == false */
            if (pdf->os_enable) {
                pdf_buffer_select(pdf, OBJSTM_BUF);
                pdf_os_write_objstream(pdf);
                pdf_flush(pdf);
                pdf_buffer_select(pdf, PDFOUT_BUF);
                /* Output the cross-reference stream dictionary */
                xref_stm = pdf_create_obj(pdf, obj_type_others, 0);
                pdf_begin_obj(pdf, xref_stm, OBJSTM_NEVER);     /* final object for pdf->os_enable == true */
                if ((obj_offset(pdf, pdf->obj_ptr) / 256) > 16777215)
                    xref_offset_width = 5;
                else if (obj_offset(pdf, pdf->obj_ptr) > 16777215)
                    xref_offset_width = 4;
                else if (obj_offset(pdf, pdf->obj_ptr) > 65535)
                    xref_offset_width = 3;
                else
                    xref_offset_width = 2;
                /* Build a linked list of free objects */
                build_free_object_list(pdf);
                pdf_begin_dict(pdf);
                pdf_dict_add_name(pdf, "Type", "XRef");
                pdf_add_name(pdf, "Index");
                pdf_begin_array(pdf);
                pdf_add_int(pdf, 0);
                pdf_add_int(pdf, pdf->obj_ptr + 1);
                pdf_end_array(pdf);
                pdf_dict_add_int(pdf, "Size", pdf->obj_ptr + 1);
                pdf_add_name(pdf, "W");
                pdf_begin_array(pdf);
                pdf_add_int(pdf, 1);
                pdf_add_int(pdf, (int) xref_offset_width);
                pdf_add_int(pdf, 1);
                pdf_end_array(pdf);
                pdf_dict_add_ref(pdf, "Root", root);
                pdf_dict_add_ref(pdf, "Info", info);
                if (pdf_trailer_toks != null) {
                    pdf_print_toks(pdf, pdf_trailer_toks);
                    delete_token_ref(pdf_trailer_toks);
                    pdf_trailer_toks = null;
                }
                print_pdf_table_string(pdf, "trailer");
                print_ID(pdf);
                pdf_dict_add_streaminfo(pdf);
                pdf_end_dict(pdf);
                pdf_begin_stream(pdf);
                for (k = 0; k <= pdf->obj_ptr; k++) {
                    if (!is_obj_written(pdf, k)) {      /* a free object */
                        pdf_out(pdf, 0);
                        pdf_out_bytes(pdf, obj_link(pdf, k), xref_offset_width);
                        pdf_out(pdf, 255);
                    } else if (obj_os_idx(pdf, k) == PDF_OS_MAX_OBJS) { /* object not in object stream */
                        pdf_out(pdf, 1);
                        pdf_out_bytes(pdf, obj_offset(pdf, k),
                                      xref_offset_width);
                        pdf_out(pdf, 0);
                    } else {    /* object in object stream */
                        pdf_out(pdf, 2);
                        pdf_out_bytes(pdf, obj_offset(pdf, k),
                                      xref_offset_width);
                        pdf_out(pdf, obj_os_idx(pdf, k));
                    }
                }
                pdf_end_stream(pdf);
                pdf_end_obj(pdf);
                pdf_flush(pdf);
            } else {
                /* Output the |obj_tab| and build a linked list of free objects */
                build_free_object_list(pdf);
                pdf_save_offset(pdf);
                pdf_puts(pdf, "xref\n");
                pdf_puts(pdf, "0 ");
                pdf_print_int_ln(pdf, pdf->obj_ptr + 1);
                pdf_print_fw_int(pdf, obj_link(pdf, 0));
                pdf_puts(pdf, " 65535 f \n");
                for (k = 1; k <= pdf->obj_ptr; k++) {
                    if (!is_obj_written(pdf, k)) {
                        pdf_print_fw_int(pdf, obj_link(pdf, k));
                        pdf_puts(pdf, " 00000 f \n");
                    } else {
                        pdf_print_fw_int(pdf, obj_offset(pdf, k));
                        pdf_puts(pdf, " 00000 n \n");
                    }
                }
            }
            /* Output the trailer */
            if (!pdf->os_enable) {
                pdf_puts(pdf, "trailer\n");
                pdf_begin_dict(pdf);
                pdf_dict_add_int(pdf, "Size", pdf->obj_ptr + 1);
                pdf_dict_add_ref(pdf, "Root", root);
                pdf_dict_add_ref(pdf, "Info", info);
                if (pdf_trailer_toks != null) {
                    pdf_print_toks(pdf, pdf_trailer_toks);
                    delete_token_ref(pdf_trailer_toks);
                    pdf_trailer_toks = null;
                }
                print_ID(pdf);
                pdf_end_dict(pdf);
                pdf_out(pdf, '\n');
            }
            pdf_puts(pdf, "startxref\n");
            pdf->cave = 0;
            if (pdf->os_enable)
                pdf_add_longint(pdf, (longinteger) obj_offset(pdf, xref_stm));
            else
                pdf_add_longint(pdf, (longinteger) pdf->save_offset);
            pdf_puts(pdf, "\n%%EOF\n");
            pdf_flush(pdf);
            if (callback_id == 0) {
                tprint_nl("Output written on ");
                tprint(pdf->file_name);
                tprint(" (");
                print_int(total_pages);
                tprint(" page");
                if (total_pages != 1)
                    print_char('s');
                tprint(", ");
                print_int(pdf_offset(pdf));
                tprint(" bytes).");
                print_ln();
            } else if (callback_id > 0) {
                run_callback(callback_id, "->");
            }
        } else {
            if (callback_id > 0) {
                run_callback(callback_id, "->");
            }
        }
        libpdffinish(pdf);
        if (pdf->draftmode == 0)
            close_file(pdf->file);
        else
            normal_warning("pdf backend","draftmode enabled, not changing output pdf");
    }
    if (callback_id == 0) {
        if (log_opened_global) {
            fprintf(log_file, "\nPDF statistics: %d PDF objects out of %d (max. %d)\n",
                (int) pdf->obj_ptr, (int) pdf->obj_tab_size,
                (int) sup_obj_tab_size);
            if (pdf->os->ostm_ctr > 0) {
                fprintf(log_file, " %d compressed objects within %d object stream%s\n",
                    (int) pdf->os->o_ctr, (int) pdf->os->ostm_ctr,
                    (pdf->os->ostm_ctr > 1 ? "s" : ""));
            }
            fprintf(log_file, " %d named destinations out of %d (max. %d)\n",
                (int) pdf->dest_names_ptr, (int) pdf->dest_names_size,
                (int) sup_dest_names_size);
            fprintf(log_file, " %d words of extra memory for PDF output out of %d (max. %d)\n",
                (int) pdf->mem_ptr, (int) pdf->mem_size,
                (int) sup_pdf_mem_size);
        }
    }
}

@ @c
void scan_pdfcatalog(PDF pdf)
{
    halfword p;
    scan_toks(false, true);
    pdf_catalog_toks = concat_tokens(pdf_catalog_toks, def_ref);
    if (scan_keyword("openaction")) {
        if (pdf_catalog_openaction != 0) {
            normal_error("pdf backend", "duplicate of openaction");
        } else {
            check_o_mode(pdf, "catalog", 1 << OMODE_PDF, true);
            p = scan_action(pdf);
            pdf_catalog_openaction = pdf_create_obj(pdf, obj_type_others, 0);
            pdf_begin_obj(pdf, pdf_catalog_openaction, OBJSTM_ALWAYS);
            write_action(pdf, p);
            pdf_end_obj(pdf);
            delete_action_ref(p);
        }
    }
}
