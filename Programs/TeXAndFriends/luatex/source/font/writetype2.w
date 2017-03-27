% writetype2.w
%
% Copyright 2006-2012 Taco Hoekwater <taco@@luatex.org>
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
#include "font/writettf.h"
#include "font/writecff.h"
#include "lua/luatex-api.h"

#include "font/sfnt.h"
#include "font/tt_glyf.h"

@ forward declaration
@c
boolean make_tt_subset(PDF pdf, fd_entry * fd, unsigned char *buff, int buflen);

@ @c
unsigned long cidtogid_obj = 0;

@ low-level helpers
@c
#define test_loc(l) \
    if ((f->loc + l) > f->buflen) { \
        normal_error("type 2","the file ended prematurely"); \
    }

BYTE get_unsigned_byte(sfnt * f)
{
    test_loc(1);
    return (BYTE) f->buffer[(f->loc++)];
}

ICHAR get_signed_byte(sfnt * f)
{
    test_loc(1);
    return (ICHAR) f->buffer[(f->loc++)];
}

USHORT get_unsigned_pair(sfnt * f)
{
    USHORT l;
    test_loc(2);
    l = f->buffer[(f->loc++)];
    l = (USHORT) (l * 0x100 + f->buffer[(f->loc++)]);
    return l;
}

SHORT get_signed_pair(sfnt * f)
{
    long l;
    test_loc(2);
    l = f->buffer[(f->loc++)];
    if (l > 0x80)
        l -= 0x100;
    l = l * 0x100 + f->buffer[(f->loc++)];
    return (SHORT) l;
}

ULONG get_unsigned_quad(sfnt * f)
{
    ULONG l;
    test_loc(4);
    l = f->buffer[(f->loc++)];
    l = l * 0x100 + f->buffer[(f->loc++)];
    l = l * 0x100 + f->buffer[(f->loc++)];
    l = l * 0x100 + f->buffer[(f->loc++)];
    return l;
}

int do_sfnt_read(unsigned char *dest, int len, sfnt * f)
{
    int i;
    test_loc(len);
    for (i = 0; i < len; i++) {
        *(dest + i) = f->buffer[f->loc + i];
    }
    f->loc += len;
    return len;
}

pdf_obj *pdf_new_stream(void)
{
    pdf_obj *stream = xmalloc(sizeof(pdf_obj));
    stream->length = 0;
    stream->data = NULL;
    return stream;
}

void pdf_add_stream(pdf_obj * stream, unsigned char *buf, long len)
{
    int i;
    assert(stream != NULL);
    if (stream->data == NULL) {
        stream->data = xmalloc((unsigned) len);
    } else {
        stream->data =
            xrealloc(stream->data, (unsigned) len + (unsigned) stream->length);
    }
    for (i = 0; i < len; i++) {
        *(stream->data + stream->length + i) = *(buf + i);
    }
    stream->length += (unsigned) len;
}

void pdf_release_obj(pdf_obj * stream)
{
    if (stream != NULL) {
        if (stream->data != NULL) {
            xfree(stream->data);
        }
        xfree(stream);
    }
}

@ The main function.
@c
boolean writetype2(PDF pdf, fd_entry * fd)
{
    int callback_id;
    int file_opened = 0;
    boolean ret;

    glyph_tab = NULL;

    fd_cur = fd;                /* |fd_cur| is global inside \.{writettf.w} */
    assert(fd_cur->fm != NULL);
    assert(is_truetype(fd_cur->fm));
    assert(is_included(fd_cur->fm));

    ttf_curbyte = 0;
    ttf_size = 0;
    cur_file_name =
        luatex_find_file(fd_cur->fm->ff_name, find_opentype_file_callback);
    if (cur_file_name == NULL) {
        formatted_error("type 2","cannot find file '%s'", fd_cur->fm->ff_name);
    }
    callback_id = callback_defined(read_opentype_file_callback);
    if (callback_id > 0) {
        if (run_callback(callback_id, "S->bSd", cur_file_name,
                         &file_opened, &ttf_buffer, &ttf_size) &&
            file_opened && ttf_size > 0) {
        } else {
            formatted_error("type 2","cannot find file '%s'", cur_file_name);
        }
    } else {
        if (!otf_open(cur_file_name)) {
            formatted_error("type 2","cannot find file '%s'", cur_file_name);
        }
        ttf_read_file();
        ttf_close();
    }

    fd_cur->ff_found = true;

    if (is_subsetted(fd_cur->fm))
        report_start_file(filetype_subset,cur_file_name);
     else
        report_start_file(filetype_font,cur_file_name);

    /* here is the real work */

    ret = make_tt_subset(pdf, fd, ttf_buffer, ttf_size);
#if 0
    xfree (dir_tab);
#endif
    xfree(ttf_buffer);
    if (is_subsetted(fd_cur->fm))
        report_stop_file(filetype_subset);
     else
        report_stop_file(filetype_font);
    cur_file_name = NULL;
    return ret;
}

@ PDF viewer applications use following tables (CIDFontType 2)

\.{head, hhea, loca, maxp, glyf, hmtx, fpgm, cvt\_, prep}

\rightline{from PDF Ref. v.1.3, 2nd ed.}

 The \.{fpgm}, \.{cvt\_} and \.{prep} tables appears only when TrueType instructions
 requires them. Those tables must be preserved if they exist.
 We use |must_exist| flag to indicate `preserve it if present'
 and to make sure not to cause an error when it does not exist.

 \.{post} and \.{name} table must exist in ordinary TrueType font file,
 but when a TrueType font is converted to CIDFontType 2 font, those tables
 are no longer required.

 The OS/2 table (required for TrueType font for Windows and OS/2) contains
 liscencing information, but PDF viewers seems not using them.

 The \.{name} table added. See comments in \.{writettf.w}.

@c
static struct {
    const char *name;
    int must_exist;
} required_table[] = {
    {
    "OS/2", 0}, {
    "cmap", 0}, {
    "head", 1}, {
    "hhea", 1}, {
    "loca", 1}, {
    "maxp", 0}, {
    "name", 1}, {
    "glyf", 1}, {
    "hmtx", 1}, {
    "fpgm", 0}, {
    "cvt ", 0}, {
    "prep", 0}, {
    NULL, 0}
};


unsigned long ttc_read_offset(sfnt * sfont, int ttc_idx, fd_entry * fd)
{
    /*ULONG version;*/
    unsigned long offset = 0;
    unsigned long num_dirs = 0;

    sfnt_seek_set(sfont, 4);    /* skip version tag */

    /*version = */(void)sfnt_get_ulong(sfont);
    num_dirs = sfnt_get_ulong(sfont);
    if (ttc_idx < 0 || ttc_idx > (int) (num_dirs - 1)) {
        formatted_error("type 2","invalid TTC index number %i (0..%i), using index 0 for font %s",
            ttc_idx,(int) (num_dirs - 1),(fd->fm->ps_name ? fd->fm->ps_name : ""));
        return 0 ;
    }
    sfnt_seek_set(sfont, 12 + ttc_idx * 4);
    offset = sfnt_get_ulong(sfont);

    return offset;
}

@ Creating the subset.
@c
extern int cidset;
boolean make_tt_subset(PDF pdf, fd_entry * fd, unsigned char *buff, int buflen)
{

    long i, cid;
    unsigned int last_cid = 0;
    glw_entry *found;
    struct avl_traverser t;
    unsigned char *cidtogidmap;
    unsigned short num_glyphs, gid;
    struct tt_glyphs *glyphs;
    char *used_chars = NULL;
    sfnt *sfont;
    pdf_obj *fontfile;
    int error = 0;

    cidtogidmap = NULL;

    sfont = sfnt_open(buff, buflen);

    if (sfont->type == SFNT_TYPE_TTC) {
        i = ff_get_ttc_index(fd->fm->ff_name, fd->fm->ps_name);
        error = sfnt_read_table_directory(sfont, ttc_read_offset(sfont, (int) i, fd));
    } else {
        error = sfnt_read_table_directory(sfont, 0);
    }

    if (error < 0) {
        normal_error("type 2","parsing the TTF directory fails");
    }

    if (sfont->type == SFNT_TYPE_TTC && sfnt_find_table_pos(sfont, "CFF ")) {
        sfnt_close(sfont);
	return false;
    }

    if (is_subsetted(fd->fm)) {
        /* rebuild the glyph tables and create a fresh cidmap */
        glyphs = tt_build_init();

        last_cid = 0;

        avl_t_init(&t, fd->gl_tree);
        for (found = (glw_entry *) avl_t_first(&t, fd->gl_tree);
             found != NULL; found = (glw_entry *) avl_t_next(&t)) {
            if (found->id > last_cid)
                last_cid = found->id;
        }

#ifndef NO_GHOSTSCRIPT_BUG
        cidtogidmap = NULL;
#else
        cidtogidmap = xmalloc(((last_cid + 1) * 2) * sizeof(unsigned char));
        memset(cidtogidmap, 0, (last_cid + 1) * 2);
#endif

        /* fill |used_chars| */
        used_chars = xmalloc((last_cid + 1) * sizeof(char));
        memset(used_chars, 0, (last_cid + 1));
        avl_t_init(&t, fd->gl_tree);
        for (found = (glw_entry *) avl_t_first(&t, fd->gl_tree);
             found != NULL; found = (glw_entry *) avl_t_next(&t)) {
            used_chars[found->id] = 1;
        }

        /* Map CIDs to GIDs. */

        num_glyphs = 1;         /* \.{.notdef} */
        for (cid = 1; cid <= (long) last_cid; cid++) {
            if (used_chars[cid] == 0)
                continue;
            gid = (short unsigned) cid;


#ifndef NO_GHOSTSCRIPT_BUG
            gid = tt_add_glyph(glyphs, (USHORT) gid, (USHORT) cid);
#else
            gid = tt_add_glyph(glyphs, (USHORT) gid, (USHORT) num_glyphs);
            cidtogidmap[2 * cid] = gid >> 8;
            cidtogidmap[2 * cid + 1] = gid & 0xff;
#endif                          /* |!NO_GHOSTSCRIPT_BUG| */

            num_glyphs++;
        }

        if (num_glyphs == 1) {
            normal_error("type 2","there are no glyphs in the subset");
        }

        if (tt_build_tables(sfont, glyphs, fd) < 0) {
            normal_error("type 2","the TTF buffer can't be parsed");
        }

        tt_build_finish(glyphs);
    }

    /* Create font file */

    for (i = 0; required_table[i].name; i++) {
        if (sfnt_require_table(sfont,required_table[i].name, required_table[i].must_exist) < 0) {
            normal_error("type 2","some required TrueType table does not exist");
        }
    }

    fontfile = sfnt_create_FontFile_stream(sfont);

    /* squeeze in the cidgidmap */
    if (cidtogidmap != NULL) {
        cidtogid_obj = (unsigned long) pdf_create_obj(pdf, obj_type_others, 0);
        pdf_begin_obj(pdf, (int) cidtogid_obj, OBJSTM_NEVER);
        pdf_begin_dict(pdf);
        pdf_dict_add_int(pdf, "Length", ((last_cid + 1) * 2));
        pdf_end_dict(pdf);
        assert(0);              /* code unused */
        pdf_begin_stream(pdf);
        pdf_room(pdf, (int) ((last_cid + 1) * 2));
        for (i = 0; i < ((int) (last_cid + 1) * 2); i++) {
            pdf_quick_out(pdf, cidtogidmap[i]);
        }
        pdf_end_stream(pdf);
        pdf_end_obj(pdf);
    }

    /* the tff subset */
    for (i = 0; i < (int) (fontfile->length); i++)
        strbuf_putchar(pdf->fb, fontfile->data[i]);

    pdf_release_obj(fontfile);

    /* CIDSet: a table of bits indexed by cid, bytes with high order bit first,
       each (set) bit is a (present) CID. */
    if (is_subsetted(fd->fm)) {
        cidset = pdf_create_obj(pdf, obj_type_others, 0);
        if (cidset != 0) {
            size_t l = (last_cid / 8) + 1;
            char *stream = xmalloc(l);
            memset(stream, 0, l);
            for (cid = 1; cid <= (long) last_cid; cid++) {
                if (used_chars[cid]) {
                    stream[(cid / 8)] |= (1 << (7 - (cid % 8)));
                }
            }
            pdf_begin_obj(pdf, cidset, OBJSTM_NEVER);
            pdf_begin_dict(pdf);
            pdf_dict_add_streaminfo(pdf);
            pdf_end_dict(pdf);
            pdf_begin_stream(pdf);
            pdf_out_block(pdf, stream, l);
            pdf_end_stream(pdf);
            pdf_end_obj(pdf);
        }
    }

    /* TODO other stuff that needs fixing: */

    /* DW, W, DW2, and W2 */
#if 0
    if (opt_flags & CIDFONT_FORCE_FIXEDPITCH) {
        pdf_add_dict(font->fontdict,
                     pdf_new_name("DW"), pdf_new_number(1000.0));
    } else {
        add_TTCIDHMetrics(font->fontdict, glyphs, used_chars, cidtogidmap,
                          last_cid);
        if (v_used_chars)
            add_TTCIDVMetrics(font->fontdict, glyphs, used_chars, cidtogidmap,
                              last_cid);
    }
#endif

    xfree(used_chars);
    sfnt_close(sfont);
    return true;
}
