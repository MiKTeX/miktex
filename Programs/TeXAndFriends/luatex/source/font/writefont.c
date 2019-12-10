/*

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2010 Taco Hoekwater <taco@luatex.org>

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

#include "ptexlib.h"
#include "lua/luatex-api.h"

int t1_wide_mode = 0 ;

void write_cid_fontdictionary(PDF pdf, fo_entry * fo, internal_font_number f);

static void create_cid_fontdictionary(PDF pdf, internal_font_number f);

const key_entry font_key[FONT_KEYS_NUM] = {
    { "Ascent",      "Ascender",    1 },
    { "CapHeight",   "CapHeight",   1 },
    { "Descent",     "Descender",   1 },
    { "ItalicAngle", "ItalicAngle", 1 },
    { "StemV",       "StdVW",       1 },
    { "XHeight",     "XHeight",     1 },
    { "FontBBox",    "FontBBox",    1 },
    { "",            "",            0 },
    { "",            "",            0 },
    { "",            "",            0 },
    { "FontName",    "FontName",    1 }
};

/*tex A tree of font dictionaries: */

struct avl_table *fo_tree = NULL;

/*tex A tree of font descriptor objects: */

struct avl_table *fd_tree = NULL;

static int comp_fo_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    return strcmp(((const fo_entry *) pa)->fm->tfm_name, ((const fo_entry *) pb)->fm->tfm_name);
}

static int comp_fd_entry(const void *pa, const void *pb, void *p)
{
    const fd_entry *p1 = (const fd_entry *) pa, *p2 = (const fd_entry *) pb;
    (void) p;
    return strcmp(p1->fm->ff_name, p2->fm->ff_name);
}

/*tex We initialize data structure for |/Type| |/Font|: */

static fo_entry *new_fo_entry(void)
{
    fo_entry *fo;
    fo = xtalloc(1, fo_entry);
    fo->fo_objnum = 0;
    fo->tex_font = 0;
    fo->fm = NULL;
    fo->fd = NULL;
    fo->fe = NULL;
    fo->cw_objnum = 0;
    fo->first_char = 1;
    fo->last_char = 0;
    fo->tx_tree = NULL;
    fo->tounicode_objnum = 0;
    return fo;
}

/*tex We initialize data structure for |/Type| |/FontDescriptor|: */

fd_entry *new_fd_entry(internal_font_number f)
{
    fd_entry *fd;
    int i;
    fd = xtalloc(1, fd_entry);
    fd->fd_objnum = 0;
    fd->fontname = NULL;
    fd->subset_tag = NULL;
    fd->ff_found = false;
    fd->ff_objnum = 0;
    fd->all_glyphs = false;
    fd->write_ttf_glyph_names = false;
    for (i = 0; i < FONT_KEYS_NUM; i++) {
        fd->font_dim[i].val = 0;
        fd->font_dim[i].set = false;
    }
    fd->fe = NULL;
    fd->builtin_glyph_names = NULL;
    fd->fm = NULL;
    fd->tx_tree = NULL;
    fd->gl_tree = NULL;
    fd->tex_font = f;
    return fd;
}

/*tex

    Only fallback values of font metrics are taken from the TFM info of |f| by
    |preset_fontmetrics|. During reading of the font file, these values are
    replaced by metrics from the font, if available.

*/

static void preset_fontmetrics(fd_entry * fd, internal_font_number f)
{
    int i;
    fd->font_dim[ITALIC_ANGLE_CODE].val = 0;
    fd->font_dim[ASCENT_CODE].val =
        divide_scaled(char_height(f, 'h'), font_size(f), 3);
    fd->font_dim[CAPHEIGHT_CODE].val =
        divide_scaled(char_height(f, 'H'), font_size(f), 3);
    i = -divide_scaled(char_depth(f, 'y'), font_size(f), 3);
    fd->font_dim[DESCENT_CODE].val = i < 0 ? i : 0;
    fd->font_dim[STEMV_CODE].val =
        divide_scaled(char_width(f, '.') / 3, font_size(f), 3);
    fd->font_dim[XHEIGHT_CODE].val =
        divide_scaled(get_x_height(f), font_size(f), 3);
    fd->font_dim[FONTBBOX1_CODE].val = 0;
    fd->font_dim[FONTBBOX2_CODE].val = fd->font_dim[DESCENT_CODE].val;
    fd->font_dim[FONTBBOX3_CODE].val =
        divide_scaled(get_quad(f), font_size(f), 3);
    fd->font_dim[FONTBBOX4_CODE].val =
        fd->font_dim[CAPHEIGHT_CODE].val > fd->font_dim[ASCENT_CODE].val ?
        fd->font_dim[CAPHEIGHT_CODE].val : fd->font_dim[ASCENT_CODE].val;
    for (i = 0; i < INT_KEYS_NUM; i++)
        fd->font_dim[i].set = true;
}

static void fix_fontmetrics(fd_entry * fd)
{
    int i;
    intparm *p = (intparm *) fd->font_dim;
    /*tex Make sure there is a rectangle. */
    if (p[FONTBBOX3_CODE].val < p[FONTBBOX1_CODE].val) {
        i = p[FONTBBOX3_CODE].val;
        p[FONTBBOX3_CODE].val = p[FONTBBOX1_CODE].val;
        p[FONTBBOX1_CODE].val = i;
    } else if (p[FONTBBOX3_CODE].val == p[FONTBBOX1_CODE].val)
        p[FONTBBOX3_CODE].val = p[FONTBBOX1_CODE].val + 1;
    if (p[FONTBBOX4_CODE].val < p[FONTBBOX2_CODE].val) {
        i = p[FONTBBOX4_CODE].val;
        p[FONTBBOX4_CODE].val = p[FONTBBOX2_CODE].val;
        p[FONTBBOX2_CODE].val = i;
    } else if (p[FONTBBOX4_CODE].val == p[FONTBBOX2_CODE].val)
        p[FONTBBOX4_CODE].val = p[FONTBBOX2_CODE].val + 1;
    if (!p[ASCENT_CODE].set) {
        p[ASCENT_CODE].val = p[FONTBBOX4_CODE].val;
        p[ASCENT_CODE].set = true;
    }
    if (!p[DESCENT_CODE].set) {
        p[DESCENT_CODE].val = p[FONTBBOX2_CODE].val;
        p[DESCENT_CODE].set = true;
    }
    if (!p[CAPHEIGHT_CODE].set) {
        p[CAPHEIGHT_CODE].val = p[FONTBBOX4_CODE].val;
        p[CAPHEIGHT_CODE].set = true;
    }
}

static void write_fontmetrics(PDF pdf, fd_entry * fd)
{
    int i;
    fix_fontmetrics(fd);
    pdf_add_name(pdf, font_key[FONTBBOX1_CODE].pdfname);
    pdf_begin_array(pdf);
    /*
    pdf_check_space;
    pdf_printf(pdf, "%i %i %i %i",
        (int) fd->font_dim[FONTBBOX1_CODE].val,
        (int) fd->font_dim[FONTBBOX2_CODE].val,
        (int) fd->font_dim[FONTBBOX3_CODE].val,
        (int) fd->font_dim[FONTBBOX4_CODE].val);
    */
    pdf_add_int(pdf,(int) fd->font_dim[FONTBBOX1_CODE].val);
    pdf_add_int(pdf,(int) fd->font_dim[FONTBBOX2_CODE].val);
    pdf_add_int(pdf,(int) fd->font_dim[FONTBBOX3_CODE].val);
    pdf_add_int(pdf,(int) fd->font_dim[FONTBBOX4_CODE].val);
    /* */
    pdf_end_array(pdf);
    for (i = 0; i < GEN_KEY_NUM; i++)
        if (fd->font_dim[i].set)
            pdf_dict_add_int(pdf, font_key[i].pdfname, fd->font_dim[i].val);
}

static void preset_fontname(fo_entry * fo, internal_font_number f)
{
    if (fo->fm->ps_name != NULL) {
        /*tex We just fallback. */
        fo->fd->fontname = xstrdup(fo->fm->ps_name);
    } else if (font_fullname(f) != NULL) {
        fo->fd->fontname = xstrdup(font_fullname(f));
    } else {
        fo->fd->fontname = xstrdup(fo->fm->tfm_name);
    }
}

static void pdf_dict_add_fontname(PDF pdf, const char *key, fd_entry * fd)
{
    char *s;
    size_t l1 = 0, l2;
    if (fd->subset_tag != NULL)
        l1 = strlen(fd->subset_tag);
    l2 = strlen(fd->fontname);
    s = xmalloc(l1 + l2 + 2);
    if (l1 > 0)
        snprintf(s, l1 + l2 + 2, "%s+%s", fd->subset_tag, fd->fontname);
    else
        snprintf(s, l2 + 1, "%s", fd->fontname);
    pdf_dict_add_name(pdf, key, s);
    xfree(s);
}

fd_entry *lookup_fd_entry(char *s)
{
    fd_entry fd;
    fm_entry fm;
    fm.ff_name = s;
    fd.fm = &fm;
    if (fd_tree == NULL) {
        fd_tree = avl_create(comp_fd_entry, NULL, &avl_xallocator);
    }
    return (fd_entry *) avl_find(fd_tree, &fd);
}

static fd_entry *lookup_fontdescriptor(fo_entry * fo)
{
    return lookup_fd_entry(fo->fm->ff_name);
}

void register_fd_entry(fd_entry * fd)
{
    void **aa;
    if (fd_tree == NULL) {
        fd_tree = avl_create(comp_fd_entry, NULL, &avl_xallocator);
    }
    /*tex The font descriptor is not yet registered: */
    if (lookup_fd_entry(fd->fm->ff_name) == NULL) {
        /*tex Is this a problem? */
    } else {
        /*tex The lookup also can create */
    }
    aa = avl_probe(fd_tree, fd);
    if (aa == NULL) {
        /*tex Is this a problem? */
    }
}

static void create_fontdescriptor(fo_entry * fo, internal_font_number f)
{
    fo->fd = new_fd_entry(f);
    preset_fontname(fo, f);
    preset_fontmetrics(fo->fd, f);
    /*tex An encoding is needed for \TRUETYPE\ writing: */
    fo->fd->fe = fo->fe;
    /*tex A map entry is needed for \TRUETYPE\ writing: */
    fo->fd->fm = fo->fm;
    fo->fd->gl_tree = avl_create(comp_string_entry, NULL, &avl_xallocator);
}

/*tex

    For all used characters of \TeX font |f|, get corresponding glyph names from
    external reencoding (.enc) file and collect these in the glyph tree |gl_tree|
    of font descriptor |fd| referenced by font dictionary |fo|.

*/

static void mark_reenc_glyphs(fo_entry * fo, internal_font_number f)
{
    int i;
    char **g;
    void **aa;
    if (is_subsetted(fo->fm)) {
        /*tex mark glyphs from TeX (externally reencoded characters) */
        g = fo->fe->glyph_names;
        for (i = fo->first_char; i <= fo->last_char; i++) {
            if (pdf_char_marked(f, i)
                && g[i] != notdef
                && (char *) avl_find(fo->fd->gl_tree, g[i]) == NULL) {
                aa = avl_probe(fo->fd->gl_tree, xstrdup(g[i]));
                if (aa == NULL) {
                    /*tex Is this a problem? */
                }
            }
        }
    }
}

/*tex

    Function |mark_chars| has 2 uses:

    \startitemize[n]
        \startitem Mark characters as chars on \TeX\ level. \stopitem
        \startitem Mark encoding pairs used by \TeX\ to optimize encoding vector. \stopitem
    \stopitemize

*/

static struct avl_table *mark_chars(fo_entry * fo, struct avl_table *tx_tree, internal_font_number f)
{
    int i, *j;
    void **aa;
    if (tx_tree == NULL) {
        tx_tree = avl_create(comp_int_entry, NULL, &avl_xallocator);
        assert(tx_tree != NULL);
    }
    for (i = fo->first_char; i <= fo->last_char; i++) {
        if (pdf_char_marked(f, i) && (int *) avl_find(tx_tree, &i) == NULL) {
            j = xtalloc(1, int);
            *j = i;
            aa = avl_probe(tx_tree, j);
            if (aa == NULL) {
                /*tex Is this a problem? */
            }
        }
    }
    return tx_tree;
}

static void get_char_range(fo_entry * fo, internal_font_number f)
{
    int i;
    assert(fo != NULL);
    /*tex Search for |first_char| and |last_char|. */
    for (i = font_bc(f); i <= font_ec(f); i++)
        if (pdf_char_marked(f, i))
            break;
    fo->first_char = i;
    for (i = font_ec(f); i >= font_bc(f); i--)
        if (pdf_char_marked(f, i))
            break;
    fo->last_char = i;
    if ((fo->first_char > fo->last_char) || !pdf_char_marked(f, fo->first_char)) {
        /*tex No character has been used from this font. */
        fo->last_char = 0;
        fo->first_char = fo->last_char + 1;
    }
}

static int font_has_subset(internal_font_number f)
{
    int i, s;
    /*tex Search for |first_char| and |last_char|. */
    for (i = font_bc(f); i <= font_ec(f); i++)
        if (pdf_char_marked(f, i))
            break;
    s = i;
    for (i = font_ec(f); i >= font_bc(f); i--)
        if (pdf_char_marked(f, i))
            break;
    if (s > i)
        return 0;
    else
        return 1;
}

static void write_charwidth_array(PDF pdf, fo_entry * fo, internal_font_number f)
{
    int i, j, *ip, *fip;
    struct avl_traverser t;
    fo->cw_objnum = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_begin_obj(pdf, fo->cw_objnum, OBJSTM_ALWAYS);
    avl_t_init(&t, fo->tx_tree);
    fip = (int *) avl_t_first(&t, fo->tx_tree);
    pdf_begin_array(pdf);
    for (ip = fip, j = *ip; ip != NULL; ip = (int *) avl_t_next(&t)) {
        if (ip != fip)
            pdf_out(pdf, ' ');
        i = *ip;
        while (j < i - 1) {
            pdf_puts(pdf, "0 ");
            j++;
        }
        j = i;
        pdf_print_charwidth(pdf, f, i);
    }
    pdf_end_array(pdf);
    pdf_end_obj(pdf);
}

/*tex

    Remark: Font objects from embedded PDF files are never registered into
    |fo_tree|; they are individually written out.

*/

static fo_entry *lookup_fo_entry(char *s)
{
    fo_entry fo;
    fm_entry fm;
    fm.tfm_name = s;
    fo.fm = &fm;
    if (fo_tree == NULL) {
        fo_tree = avl_create(comp_fo_entry, NULL, &avl_xallocator);
    }
    return (fo_entry *) avl_find(fo_tree, &fo);
}

static void register_fo_entry(fo_entry * fo)
{
    void **aa;
    if (fo_tree == NULL) {
        fo_tree = avl_create(comp_fo_entry, NULL, &avl_xallocator);
    }
    if (lookup_fo_entry(fo->fm->tfm_name) == NULL) {
        /*tex Is this a problem? */
    } else {
        /*tex The lookup also can create */
    }
    aa = avl_probe(fo_tree, fo);
    if (aa == NULL) {
        /*tex Is this a problem? */
    }
}

/*tex

In principle we could replace the pdftex derived ttf.otf inclusion part by using
the regular code for this and assigning indices and tounicodes to the character
blobs, but for the moment we keep the current approach.

*/

static void write_fontfile(PDF pdf, fd_entry * fd)
{
    if (is_cidkeyed(fd->fm)) {
        if (is_opentype(fd->fm)) {
            writetype0(pdf, fd);
        } else if (is_truetype(fd->fm)) {
            if (!writetype2(pdf, fd)) {
                writetype0(pdf,fd);
                fd->fm->type |= F_OTF; fd->fm->type ^= F_TRUETYPE;
            }
        } else if (is_type1(fd->fm)) {
            if (t1_wide_mode) {
                writet1(pdf, fd, 1);
            } else {
                writetype1w(pdf, fd);
            }
        } else {
            normal_error("fonts","there is a problem writing the font file (1)");
        }
    } else {
        if (is_type1(fd->fm)) {
            writet1(pdf, fd, 0);
        } else if (is_truetype(fd->fm)) {
            writettf(pdf, fd);
        } else if (is_opentype(fd->fm)) {
            writeotf(pdf, fd);
        } else {
            normal_error("fonts","there is a problem writing the font file (2)");
        }
    }
    if (!fd->ff_found)
        return;
    fd->ff_objnum = pdf_create_obj(pdf, obj_type_others, 0);
    /*tex The font file stream: */
    pdf_begin_obj(pdf, fd->ff_objnum, OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    if (is_cidkeyed(fd->fm)) {
        /*tex No subtype is used for |TRUETYPE\ based \OPENTYPE\ fonts. */
        if (is_opentype(fd->fm)) {
            pdf_dict_add_name(pdf, "Subtype", "CIDFontType0C");
        } else if (is_type1(fd->fm)) {
            if (t1_wide_mode) {
                pdf_dict_add_int(pdf, "Length1", (int) t1_length1);
                pdf_dict_add_int(pdf, "Length2", (int) t1_length2);
                pdf_dict_add_int(pdf, "Length3", (int) t1_length3);
            } else {
                pdf_dict_add_name(pdf, "Subtype", "CIDFontType0C");
            }
        }
    } else if (is_type1(fd->fm)) {
        pdf_dict_add_int(pdf, "Length1", (int) t1_length1);
        pdf_dict_add_int(pdf, "Length2", (int) t1_length2);
        pdf_dict_add_int(pdf, "Length3", (int) t1_length3);
    } else if (is_truetype(fd->fm)) {
        pdf_dict_add_int(pdf, "Length1", (int) ttf_length);
    } else if (is_opentype(fd->fm)) {
        pdf_dict_add_name(pdf, "Subtype", "Type1C");
    } else {
        normal_error("fonts","there is a problem writing the font file (3)");
    }
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    strbuf_flush(pdf, pdf->fb);
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
}

int cidset = 0;

static void write_fontdescriptor(PDF pdf, fd_entry * fd)
{
    static const int std_flags[] = {
        1 + 2 + (1 << 5),                        /* Courier */
        1 + 2 + (1 << 5)            + (1 << 18), /* Courier-Bold */
        1 + 2 + (1 << 5) + (1 << 6),             /* Courier-Oblique */
        1 + 2 + (1 << 5) + (1 << 6) + (1 << 18), /* Courier-BoldOblique */
                (1 << 5),                        /* Helvetica */
                (1 << 5)            + (1 << 18), /* Helvetica-Bold */
                (1 << 5) + (1 << 6),             /* Helvetica-Oblique */
                (1 << 5) + (1 << 6) + (1 << 18), /* Helvetica-BoldOblique */
              4,                                 /* Symbol */
            2 + (1 << 5),                        /* Times-Roman */
            2 + (1 << 5)            + (1 << 18), /* Times-Bold */
            2 + (1 << 5) + (1 << 6),             /* Times-Italic */
            2 + (1 << 5) + (1 << 6) + (1 << 18), /* Times-BoldItalic */
              4                                  /* ZapfDingbats */
    };
    char *glyph;
    struct avl_traverser t;
    int fd_flags;
    /*tex Possibly updated by |write_fontfile|: */
    cidset = 0;
    if (fd->fd_objnum == 0) {
        int n = 0;
        int callback_id = callback_defined(font_descriptor_objnum_provider_callback);
        if (callback_id) {
            run_callback(callback_id, "S->d", fd->fontname, &n);
        }
        if (!n) {
           n = pdf_create_obj(pdf, obj_type_others, 0);
        }
        fd->fd_objnum = n;
    }
    if (is_fontfile(fd->fm) && is_included(fd->fm)) {
        /*tex This will set |fd->ff_found| if font file is found: */
        write_fontfile(pdf, fd);
    }
    pdf_begin_obj(pdf, fd->fd_objnum, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "FontDescriptor");
    pdf_dict_add_fontname(pdf, "FontName", fd);
    if (fd->fm->fd_flags != FD_FLAGS_NOT_SET_IN_MAPLINE) {
        fd_flags = (int) fd->fm->fd_flags;
    } else if (fd->ff_found) {
        fd_flags = FD_FLAGS_DEFAULT_EMBED;
    } else {
        fd_flags = is_std_t1font(fd->fm) ? std_flags[check_std_t1font(fd->fm->ps_name)] : FD_FLAGS_DEFAULT_NON_EMBED;
        formatted_warning("map file",
             "No flags specified for non-embedded font '%s' (%s), I'm using %i, fix your map entry",
             fd->fm->ps_name != NULL ? fd->fm->ps_name : "No name given",
             fd->fm->tfm_name, fd_flags);
    }
    pdf_dict_add_int(pdf, "Flags", fd_flags);
    write_fontmetrics(pdf, fd);
    if (fd->ff_found) {
        if (is_cidkeyed(fd->fm)) {
            if (is_type1(fd->fm)) {
                if (t1_wide_mode) {
                    pdf_dict_add_ref(pdf, "FontFile", (int) fd->ff_objnum);
                } else {
                    pdf_dict_add_ref(pdf, "FontFile3", (int) fd->ff_objnum);
                }
            } else if (is_truetype(fd->fm)) {
                pdf_dict_add_ref(pdf, "FontFile2", (int) fd->ff_objnum);
            } else if (is_opentype(fd->fm)) {
                pdf_dict_add_ref(pdf, "FontFile3", (int) fd->ff_objnum);
            } else {
                normal_error("fonts","there is a problem writing the font file (4)");
            }
        } else {
            if (is_subsetted(fd->fm) && is_type1(fd->fm)) {
                /*tex |/CharSet| is optional; names may appear in any order */
                if ((! pdf->omit_charset) && (pdf->major_version == 1)) {
                    avl_t_init(&t, fd->gl_tree);
                    pdf_add_name(pdf, "CharSet");
                    pdf_out(pdf, '(');
                    for (glyph = (char *) avl_t_first(&t, fd->gl_tree); glyph != NULL; glyph = (char *) avl_t_next(&t)) {
                        pdf_add_name(pdf, glyph);
                    }
                    pdf_out(pdf, ')');
                    pdf_set_space(pdf);
                }
            }
            if (is_type1(fd->fm)) {
                pdf_dict_add_ref(pdf, "FontFile", (int) fd->ff_objnum);
            } else if (is_truetype(fd->fm)) {
                pdf_dict_add_ref(pdf, "FontFile2", (int) fd->ff_objnum);
            } else if (is_opentype(fd->fm)) {
                pdf_dict_add_ref(pdf, "FontFile3", (int) fd->ff_objnum);
            } else {
                normal_error("fonts","there is a problem writing the font file (5)");
            }
        }
    }
    if ((! pdf->omit_cidset) && (pdf->major_version == 1) && (cidset != 0) ) {
        pdf_dict_add_ref(pdf, "CIDSet", cidset);
    }
    /*tex
        Currently we don't export the optional keys for CID fonts like |/Style <<
        /Panose <12-byte string> >>| and we probably never will.
    */
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
}

static void write_fontdescriptors(PDF pdf)
{
    fd_entry *fd;
    struct avl_traverser t;
    if (fd_tree == NULL)
        return;
    avl_t_init(&t, fd_tree);
    for (fd = (fd_entry *) avl_t_first(&t, fd_tree); fd != NULL; fd = (fd_entry *) avl_t_next(&t))
        write_fontdescriptor(pdf, fd);
}

static void write_fontdictionary(PDF pdf, fo_entry * fo)
{
    /*tex Write the |/ToUnicode| entry if needed. */
    if (pdf->gen_tounicode > 0 && fo->fd != NULL) {
        if (fo->fe != NULL) {
            fo->tounicode_objnum = write_tounicode(pdf, fo->fe->glyph_names, fo->fe->name);
        } else if (is_type1(fo->fm)) {
            fo->tounicode_objnum = write_tounicode(pdf, fo->fd->builtin_glyph_names, fo->fm->tfm_name);
        }
    }
    pdf_begin_obj(pdf, fo->fo_objnum, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "Font");
    if (is_type1(fo->fm)) {
        pdf_dict_add_name(pdf, "Subtype", "Type1");
    } else if (is_truetype(fo->fm)) {
        pdf_dict_add_name(pdf, "Subtype", "TrueType");
    } else if (is_opentype(fo->fm)) {
        pdf_dict_add_name(pdf, "Subtype", "Type1");
    } else {
        normal_error("fonts","there is a problem writing the font file (6)");
    }
    pdf_dict_add_fontname(pdf, "BaseFont", fo->fd);
    pdf_dict_add_ref(pdf, "FontDescriptor", (int) fo->fd->fd_objnum);
    pdf_dict_add_int(pdf, "FirstChar", (int) fo->first_char);
    pdf_dict_add_int(pdf, "LastChar", (int) fo->last_char);
    pdf_dict_add_ref(pdf, "Widths", (int) fo->cw_objnum);
    if ((is_type1(fo->fm) || is_opentype(fo->fm)) && fo->fe != NULL && fo->fe->fe_objnum != 0) {
        pdf_dict_add_ref(pdf, "Encoding", (int) fo->fe->fe_objnum);
    }
    if (fo->tounicode_objnum != 0) {
        pdf_dict_add_ref(pdf, "ToUnicode", (int) fo->tounicode_objnum);
    }
    if (pdf_font_attr(fo->tex_font) != get_nullstr() && pdf_font_attr(fo->tex_font) != 0) {
        pdf_check_space(pdf);
        pdf_print(pdf, pdf_font_attr(fo->tex_font));
        pdf_set_space(pdf);
    }
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
}

static void write_fontdictionaries(PDF pdf)
{
    fo_entry *fo;
    struct avl_traverser t;
    if (fo_tree == NULL)
        return;
    avl_t_init(&t, fo_tree);
    for (fo = (fo_entry *) avl_t_first(&t, fo_tree); fo != NULL; fo = (fo_entry *) avl_t_next(&t)) {
        write_fontdictionary(pdf, fo);
    }
}

/*tex

    Final flush of all font related stuff by call from \.{Output fonts
    definitions} elsewhere.

*/

void write_fontstuff(PDF pdf)
{
    write_fontdescriptors(pdf);
    write_fontencodings(pdf);
    write_fontdictionaries(pdf);
}

static void create_fontdictionary(PDF pdf, internal_font_number f)
{
    fo_entry *fo = new_fo_entry();
    fm_entry *fm = font_map(f);
    /*tex set |fo->first_char| and |fo->last_char| from |f| */
    get_char_range(fo, f);
    if (fo->last_char > 255)
        fo->last_char = 255;
    fo->fm = fm;
    fo->fo_objnum = pdf_font_num(f);
    fo->tex_font = f;
    if (is_reencoded(fo->fm)) {
        /*tex
            At least the map entry tells so but it returns |NULL| if the .enc
            file couldn't be opened.
        */
        fo->fe = get_fe_entry(fo->fm->encname);
        if (fo->fe != NULL && (is_type1(fo->fm) || is_opentype(fo->fm))) {
            /*tex We don't end up here for truetype fonts. */
            if (fo->fe->fe_objnum == 0) {
                /*tex It will be written out */
                fo->fe->fe_objnum = pdf_create_obj(pdf, obj_type_others, 0);
            }
            /*tex Mark encoding pairs used by TeX to optimize encoding vector. */
            fo->fe->tx_tree = mark_chars(fo, fo->fe->tx_tree, f);
        }
    }
    /*tex For |write_charwidth_array|: */
    fo->tx_tree = mark_chars(fo, fo->tx_tree, f);
    write_charwidth_array(pdf, fo, f);
    if (!is_builtin(fo->fm)) {
        if (is_type1(fo->fm)) {
            if ((fo->fd = lookup_fontdescriptor(fo)) == NULL) {
                create_fontdescriptor(fo, f);
                register_fd_entry(fo->fd);
            }
        } else {
            create_fontdescriptor(fo, f);
        }
        if (fo->fe != NULL) {
            mark_reenc_glyphs(fo, f);
            if (!is_type1(fo->fm)) {
                /*tex Mark reencoded characters as chars on TeX level. */
                assert(fo->fd->tx_tree == NULL);
                fo->fd->tx_tree = mark_chars(fo, fo->fd->tx_tree, f);
                if (is_truetype(fo->fm)) {
                    fo->fd->write_ttf_glyph_names = true;
                }
            }
        } else {
            /*tex Mark non-reencoded characters as chars on TeX level. */
            fo->fd->tx_tree = mark_chars(fo, fo->fd->tx_tree, f);
        }
        if (!is_type1(fo->fm)) {
            write_fontdescriptor(pdf, fo->fd);
        }
    } else {
        /*tex
            Builtin fonts still need the \type {/Widths} array and \type
            {/FontDescriptor} (to avoid error \quotation {font FOO contains bad
            \type {/BBox}}).
        */
        create_fontdescriptor(fo, f);
        write_fontdescriptor(pdf, fo->fd);
        if (!is_std_t1font(fo->fm)) {
            formatted_warning("map file", "font '%s' is not a standard font; I suppose it is available to your PDF viewer then", fo->fm->ps_name);
        }
    }
    if (is_type1(fo->fm)) {
        register_fo_entry(fo);
    } else {
        write_fontdictionary(pdf, fo);
    }
}

static int has_ttf_outlines(fm_entry * fm)
{
    FILE *f = fopen(fm->ff_name, "rb");
    if (f != NULL) {
        int ch1 = getc(f);
        int ch2 = getc(f);
        int ch3 = getc(f);
        int ch4 = getc(f);
        fclose(f);
        if (ch1 == 'O' && ch2 == 'T' && ch3 == 'T' && ch4 == 'O')
            return 0;
        return 1;
    }
    return 0;
}

void do_pdf_font(PDF pdf, internal_font_number f)
{
    fm_entry *fm;
    /*tex
        This is not 100\% true: CID is actually needed whenever (and only) there
        are more than 256 separate glyphs used. But for now, we just assume the
        user knows what he is doing. In practice this seems to be the case.
    */
    if (!font_has_subset(f))
        return;
    if (font_encodingbytes(f) == 2) {
        /*tex
            Create a virtual font map entry, as this is needed by the rest of the
            font inclusion mechanism.
        */
        fm = font_map(f) = new_fm_entry();
        /*tex Set this to a name or whatever, not a real \TFM\ anyway: */
        fm->tfm_name = font_name(f);
        /*tex The actual file: */
        fm->ff_name = font_filename(f);
        /*tex The true (used) name: */
        if (font_psname(f) != NULL) {
            fm->ps_name = font_psname(f);
        } else {
            fm->ps_name = font_fullname(f);
        }
        /*tex Needed for the CIDSystemInfo: */
        fm->encname = font_encodingname(f);
        fm->subfont = font_subfont(f);
        fm->slant = font_slant(f);
        set_slantset(fm);
        fm->extend = font_extend(f);
        set_extendset(fm);
        /*tex Flags can perhaps be done better. */
        fm->fd_flags = 4;
        set_inuse(fm);
        switch (font_format(f)) {
            case opentype_format:
                if (has_ttf_outlines(fm)) {
                    set_truetype(fm);
                } else {
                    set_opentype(fm);
                }
                break;
            case truetype_format:
                set_truetype(fm);
                break;
            case type1_format:
                set_type1(fm);
                break;
            default:
                formatted_error("font","file format '%s' for '%s' is incompatible with wide characters",
                    font_format_name(f), font_name(f));
        }
        /*tex This makes \quotation {unknown} default to subsetted inclusion. */
        if (font_embedding(f) != no_embedding) {
            set_included(fm);
            if (font_embedding(f) != full_embedding) {
                set_subsetted(fm);
            }
        }
        set_cidkeyed(fm);
        create_cid_fontdictionary(pdf, f);
    } else {
        /*tex By now |font_map(f)|, if any, should have been set via |pdf_init_font|. */
        if ((fm = font_map(f)) == NULL || (fm->ps_name == NULL && fm->ff_name == NULL))
            writet3(pdf, f);
        else
            create_fontdictionary(pdf, f);
    }
}

/*tex

    The glyph width is included in |glw_entry|, because that width depends on the
    value it has in the font where it is actually typeset from, not the font that
    is the owner of the fd entry.

    It is possible that the user messes with the metric width, but handling that
    properly would require access to the |hmtx| table at this point in the
    program.

*/

static int comp_glw_entry(const void *pa, const void *pb, void *p __attribute__ ((unused)))
{
    unsigned short i, j;
    i = (unsigned short) (*(const glw_entry *) pa).id;
    j = (unsigned short) (*(const glw_entry *) pb).id;
    cmp_return(i, j);
    return 0;
}

static void create_cid_fontdescriptor(fo_entry * fo, internal_font_number f)
{
    fo->fd = new_fd_entry(f);
    preset_fontname(fo, f);
    preset_fontmetrics(fo->fd, f);
    /*tex Encoding needed by \TRUETYPE\ writing: */
    fo->fd->fe = fo->fe;
    /*tex Map entry needed by \TRUETYPE\ writing */
    fo->fd->fm = fo->fm;
    fo->fd->gl_tree = avl_create(comp_glw_entry, NULL, &avl_xallocator);
    assert(fo->fd->gl_tree != NULL);
}

/*tex

    The values |font_bc()| and |font_ec()| are potentially large character ids,
    but the strings that are written out use CID indexes, and those are limited
    to 16-bit values.

*/

static void mark_cid_subset_glyphs(fo_entry * fo, internal_font_number f)
{
    glw_entry *j;
    void *aa;
    int l = font_size(f);
    int i;
    for (i = font_bc(f); i <= font_ec(f); i++) {
        if (quick_char_exists(f, i) && char_used(f, i)) {
            j = xtalloc(1, glw_entry);
            j->id = (unsigned) char_index(f, i);
            j->wd = divide_scaled_n(char_width(f, i), l, 10000.0);
            if ((glw_entry *) avl_find(fo->fd->gl_tree, j) == NULL) {
                aa = avl_probe(fo->fd->gl_tree, j);
                if (aa == NULL) {
                    /*tex Is this a problem? */
                }
            } else {
                xfree(j);
            }
        }
    }
}

/*tex

    It is possible to compress the widths array even better, by using the
    alternate 'range' syntax and possibly even using /DW to set a default value.

    There is a some optimization here already: glyphs that are not used do not
    appear in the widths array at all.

    We have to make sure that we do not output an (incorrect!) width for a
    character that exists in the font, but is not used in typesetting. An
    enormous negative width is used as sentinel value

*/

static void write_cid_charwidth_array(PDF pdf, fo_entry * fo)
{
    int i, j;
    glw_entry *glyph;
    struct avl_traverser t;
    fo->cw_objnum = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_begin_obj(pdf, fo->cw_objnum, OBJSTM_ALWAYS);
    avl_t_init(&t, fo->fd->gl_tree);
    glyph = (glw_entry *) avl_t_first(&t, fo->fd->gl_tree);
    i = (int) glyph->id;
    pdf_begin_array(pdf);
    pdf_add_int(pdf, i);
    pdf_begin_array(pdf);
    for (; glyph != NULL; glyph = (glw_entry *) avl_t_next(&t)) {
        j = glyph->wd;
        if (glyph->id > (unsigned) (i + 1)) {
            pdf_end_array(pdf);
            pdf_add_int(pdf, glyph->id);
            pdf_begin_array(pdf);
            j = glyph->wd;
        }
        pdf_check_space(pdf);
        if (j < 0) {
            pdf_out(pdf, '-');
            j = -j;
        }
        pdf_printf(pdf, "%i", (j / 10));
        if ((j % 10) != 0)
            pdf_printf(pdf, ".%i", (j % 10));
        i = (int) glyph->id;
        pdf_set_space(pdf);
    }
    pdf_end_array(pdf);
    pdf_end_array(pdf);
    pdf_end_obj(pdf);
}

static void destroy_glw_cid_entry(void *pa, void *pb)
{
    glw_entry *e = (glw_entry *) pa;
    (void) pb;
    xfree(e);
}

static void create_cid_fontdictionary(PDF pdf, internal_font_number f)
{
    fm_entry *fm = font_map(f);
    fo_entry *fo = new_fo_entry();
    /*tex set |fo->first_char| and |fo->last_char| from |f| */
    get_char_range(fo, f);
    fo->fm = fm;
    fo->fo_objnum = pdf_font_num(f);
    fo->tex_font = f;
    create_cid_fontdescriptor(fo, f);
    mark_cid_subset_glyphs(fo, f);
    if (is_subsetted(fo->fm)) {
        /*tex
           This is a bit sneaky. |make_subset_tag()| actually expects the glyph
           tree to contain strings instead of |glw_entry| items. However, all
           calculations are done using explicit typecasts, so it works out ok.
        */
        make_subset_tag(fo->fd);
    }
    write_cid_charwidth_array(pdf, fo);
    write_fontdescriptor(pdf, fo->fd);
    write_cid_fontdictionary(pdf, fo, f);
    if (fo->fd) {
        if (fo->fd->gl_tree) {
            avl_destroy(fo->fd->gl_tree,destroy_glw_cid_entry);
        }
        xfree(fo->fd);
    }
    xfree(fo);
}

void write_cid_fontdictionary(PDF pdf, fo_entry * fo, internal_font_number f)
{
    int i;
    fo->tounicode_objnum = write_cid_tounicode(pdf, fo, f);
    pdf_begin_obj(pdf, fo->fo_objnum, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "Font");
    pdf_dict_add_name(pdf, "Subtype", "Type0");
    if (font_identity(f) == vertical_identity) {
        pdf_dict_add_name(pdf, "Encoding", "Identity-V");
    } else {
        pdf_dict_add_name(pdf, "Encoding", "Identity-H");
    }
    pdf_dict_add_fontname(pdf, "BaseFont", fo->fd);
    i = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_add_name(pdf, "DescendantFonts");
    pdf_begin_array(pdf);
    pdf_add_ref(pdf, i);
    pdf_end_array(pdf);
    if (fo->tounicode_objnum != 0)
        pdf_dict_add_ref(pdf, "ToUnicode", (int) fo->tounicode_objnum);
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
    pdf_begin_obj(pdf, i, OBJSTM_ALWAYS);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "Font");
    if (is_opentype(fo->fm)) {
        pdf_dict_add_name(pdf, "Subtype", "CIDFontType0");
    } else if (is_type1(fo->fm)) {
        if (t1_wide_mode) {
            pdf_dict_add_name(pdf, "Subtype", "CIDFontType0");
         // pdf_dict_add_name(pdf, "CIDToGIDMap", "Identity");
        } else {
            pdf_dict_add_name(pdf, "Subtype", "CIDFontType0");
        }
    } else {
        pdf_dict_add_name(pdf, "Subtype", "CIDFontType2");
        pdf_dict_add_name(pdf, "CIDToGIDMap", "Identity");
    }
    pdf_dict_add_fontname(pdf, "BaseFont", fo->fd);
    pdf_dict_add_ref(pdf, "FontDescriptor", (int) fo->fd->fd_objnum);
    pdf_dict_add_ref(pdf, "W", (int) fo->cw_objnum);
    pdf_add_name(pdf, "CIDSystemInfo");
    pdf_begin_dict(pdf);
    pdf_dict_add_string(pdf, "Registry", (font_cidregistry(f) ? font_cidregistry(f) : "Adobe"));
    pdf_dict_add_string(pdf, "Ordering", (font_cidordering(f) ? font_cidordering(f) : "Identity"));
    pdf_dict_add_int(pdf, "Supplement", (int) font_cidsupplement(f));
    pdf_end_dict(pdf);
    /*tex
        I doubt there is anything useful that could be written here so for now we
        comment this.
    */
    /*
        if (pdf_font_attr(fo->tex_font) != get_nullstr()) {
            pdf_out(pdf, '\n');
            pdf_print(pdf_font_attr(fo->tex_font));
            pdf_out(pdf, '\n');
        }
    */
    pdf_end_dict(pdf);
    pdf_end_obj(pdf);
}
